#include "timebox/host.hpp"
#include "timebox/mcp.hpp"

#include "graphics/graphics.hpp"
#include "graphics/timebox_timer.hpp"

#include "json/json.h"

#include <chrono>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

namespace {

int g_failures = 0;

void expect(bool cond, const std::string &message) {
  if (!cond) {
    std::cerr << "FAIL " << message << "\n";
    ++g_failures;
  }
}

std::string dump(const Json::Value &value) {
  Json::StreamWriterBuilder builder;
  builder["indentation"] = "";
  std::string text = Json::writeString(builder, value);
  while (!text.empty() && (text.back() == '\n' || text.back() == '\r'))
    text.pop_back();
  return text;
}

bool parse_json(const std::string &text, Json::Value &root) {
  Json::CharReaderBuilder builder;
  const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  std::string error;
  return reader->parse(text.data(), text.data() + text.size(), &root, &error);
}

std::vector<uint8_t> timer_frame(int remaining_ms, int total_ms) {
  graphics::GraphicsContext ctx(LED_WIDTH, LED_HEIGHT);
  graphics::paint_timebox_timer(ctx, remaining_ms, total_ms);
  const auto &px = ctx.get_pixels();
  std::vector<uint8_t> frame(static_cast<size_t>(TimeboxHost::kFrameBytes));
  for (size_t i = 0; i < px.size(); ++i) {
    frame[i * 3] = px[i].r;
    frame[i * 3 + 1] = px[i].g;
    frame[i * 3 + 2] = px[i].b;
  }
  return frame;
}

std::string base64_encode(const uint8_t *data, size_t n) {
  static const char *table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string out;
  out.reserve((n + 2) / 3 * 4);
  for (size_t i = 0; i + 3 <= n; i += 3) {
    const unsigned value = (static_cast<unsigned>(data[i]) << 16) |
                           (static_cast<unsigned>(data[i + 1]) << 8) |
                           static_cast<unsigned>(data[i + 2]);
    out.push_back(table[(value >> 18) & 63]);
    out.push_back(table[(value >> 12) & 63]);
    out.push_back(table[(value >> 6) & 63]);
    out.push_back(table[value & 63]);
  }
  return out;
}

TimeboxProcess find_name(const std::vector<TimeboxProcess> &list, const std::string &name) {
  for (const auto &process : list) {
    if (process.name == name)
      return process;
  }
  return {};
}

struct ToolResult {
  bool ok = false;
  bool is_error = false;
  std::string error;
  Json::Value payload;
};

ToolResult call_tool(TimeboxHost &host, const std::string &name, const Json::Value &args) {
  Json::Value request;
  request["jsonrpc"] = "2.0";
  request["id"] = 7;
  request["method"] = "tools/call";
  request["params"]["name"] = name;
  request["params"]["arguments"] = args;
  const std::string response = handle_mcp_message(host, dump(request));
  ToolResult result;
  Json::Value root;
  if (!parse_json(response, root) || !root.isMember("result")) {
    result.error = "bad rpc response: " + response.substr(0, 180);
    return result;
  }
  result.ok = true;
  result.is_error = root["result"]["isError"].asBool();
  const std::string text = root["result"]["content"][0]["text"].asString();
  if (result.is_error) {
    result.error = text;
    return result;
  }
  if (!parse_json(text, result.payload))
    result.error = "tool text is not json";
  return result;
}

void test_two_processes_focus_and_paint() {
  TimeboxHost host;
  TimeboxProcess meeting;
  TimeboxProcess splash;
  const std::string started =
      host.start_process("timer", "meeting", 10, true, &meeting);
  expect(started.empty(), "start timer: " + started);
  const std::string painted =
      host.start_process("paint", "splash", -1, false, &splash);
  expect(painted.empty(), "start paint: " + painted);
  expect(host.list_processes().size() == 2, "two processes are running");
  expect(host.status().focused_name == "meeting", "timer starts focused");
  expect(host.status().focused_id == meeting.id, "focus id is the timer");

  std::vector<uint8_t> marker(static_cast<size_t>(TimeboxHost::kFrameBytes), 0);
  const size_t lid = (static_cast<size_t>(0) * TimeboxHost::kWidth + 2 * 64) * 3;
  marker[lid] = 0;
  marker[lid + 1] = 220;
  marker[lid + 2] = 40;
  TimeboxProcess updated;
  const std::string paint_error =
      host.paint_frame(splash.id, marker.data(), marker.size(), &updated);
  expect(paint_error.empty(), "paint_frame: " + paint_error);
  expect(!updated.focused, "paint_frame does not steal focus");
  expect(host.status().focused_name == "meeting", "focus stays on the timer");

  std::vector<uint8_t> splash_frame;
  TimeboxProcess splash_info;
  expect(host.snapshot(0, "splash", &splash_frame, &splash_info).empty(), "snapshot splash");
  expect(splash_frame == marker, "paint_frame updates the unfocused buffer");

  std::vector<uint8_t> focused;
  TimeboxProcess focused_info;
  expect(host.snapshot(0, "", &focused, &focused_info).empty(), "snapshot focused");
  expect(focused_info.name == "meeting", "empty snapshot reads the focus");
  expect(focused == timer_frame(10000, 10000), "timer slot uses the OG faces at start");

  host.advance_timers(5000);
  TimeboxProcess meeting_after = find_name(host.list_processes(), "meeting");
  expect(meeting_after.remaining_ms == 5000, "unfocused path is not required for the timer to run");
  expect(host.snapshot(meeting.id, "", &focused, &focused_info).empty(), "snapshot timer by id");
  expect(focused == timer_frame(5000, 10000), "OG faces follow the remaining time");
  expect(host.snapshot(0, "splash", &splash_frame, &splash_info).empty(), "splash still there");
  expect(splash_frame == marker, "timer ticks do not clobber a paint slot");

  TimeboxProcess shown;
  expect(host.show_process(0, "splash", &shown).empty(), "show by name");
  expect(shown.focused && host.status().focused_id == splash.id, "show switches focus");
  expect(host.list_processes().size() == 2, "show does not stop the timer");
  host.advance_timers(1000);
  meeting_after = find_name(host.list_processes(), "meeting");
  expect(meeting_after.remaining_ms == 4000, "unfocused timer keeps running");
  expect(host.show_process(meeting.id, "", &shown).empty(), "show by id");
  expect(host.status().focused_name == "meeting", "show by id focuses the timer");

  const std::string not_paint =
      host.paint_frame(meeting.id, marker.data(), marker.size(), nullptr);
  expect(!not_paint.empty(), "paint_frame rejects a timer");
  const std::string short_frame = host.paint_frame(splash.id, marker.data(), 3, nullptr);
  expect(!short_frame.empty(), "paint_frame rejects a short buffer");

  TimeboxProcess stopped;
  expect(host.stop_process(0, "splash", &stopped).empty(), "stop splash");
  expect(host.list_processes().size() == 1, "timer still running after stop");
  expect(find_name(host.list_processes(), "meeting").id == meeting.id, "meeting remains");

  TimeboxProcess extra;
  expect(host.start_process("timer", "meeting", 5, false, &extra) == "name already in use",
         "names are unique");

  int started_ok = 1;  // meeting is still alive
  for (int i = 0; i < TimeboxHost::kCapacity - 1; ++i) {
    const std::string name = "p" + std::to_string(i);
    if (!host.start_process("paint", name, -1, false, nullptr).empty())
      started_ok = 0;
  }
  expect(started_ok == 1, "fills the table up to 8");
  expect(host.list_processes().size() == static_cast<size_t>(TimeboxHost::kCapacity),
         "capacity is 8");
  expect(host.start_process("paint", "overflow", -1, true, nullptr).find("full") != std::string::npos,
         "ninth process is rejected");
}

void test_mcp_tools() {
  TimeboxHost host;
  const std::string init = handle_mcp_message(
      host, R"({"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2024-11-05","capabilities":{}}})");
  expect(init.find("\"name\":\"timebox\"") != std::string::npos, "initialize names the server");
  expect(handle_mcp_message(host, R"({"jsonrpc":"2.0","method":"notifications/initialized"})").empty(),
         "initialized notification has no response");

  const std::string tools = handle_mcp_message(host, R"({"jsonrpc":"2.0","id":2,"method":"tools/list"})");
  const char *names[] = {
      "timebox_status", "timebox_list_processes", "timebox_start", "timebox_stop",
      "timebox_show",   "timebox_paint_frame",    "timebox_snapshot"};
  for (const char *name : names)
    expect(tools.find(std::string("\"") + name + "\"") != std::string::npos,
           std::string("tools/list includes ") + name);

  Json::Value start_timer;
  start_timer["kind"] = "timer";
  start_timer["name"] = "meeting";
  ToolResult timer = call_tool(host, "timebox_start", start_timer);
  expect(!timer.is_error && timer.payload["remaining_ms"].asInt() == 60000,
         "omitted seconds defaults to 60: " + timer.error);
  expect(timer.payload["focused"].asBool(), "focus defaults to true");

  Json::Value start_paint;
  start_paint["kind"] = "paint";
  start_paint["name"] = "splash";
  start_paint["focus"] = false;
  ToolResult paint = call_tool(host, "timebox_start", start_paint);
  expect(!paint.is_error, "start paint: " + paint.error);

  ToolResult listed = call_tool(host, "timebox_list_processes", Json::Value(Json::objectValue));
  expect(!listed.is_error && listed.payload["processes"].size() == 2, "MCP lists both processes");

  std::vector<uint8_t> marker(static_cast<size_t>(TimeboxHost::kFrameBytes), 1);
  marker[0] = 9;
  marker[1] = 8;
  marker[2] = 7;
  const std::string encoded = base64_encode(marker.data(), marker.size());
  Json::Value paint_args;
  paint_args["process_id"] = paint.payload["id"].asInt();
  paint_args["frame_base64"] = encoded;
  ToolResult painted = call_tool(host, "timebox_paint_frame", paint_args);
  expect(!painted.is_error && !painted.payload["focused"].asBool(),
         "MCP paint while unfocused: " + painted.error);

  Json::Value snap_args;
  snap_args["name"] = "splash";
  ToolResult snap = call_tool(host, "timebox_snapshot", snap_args);
  expect(!snap.is_error && snap.payload["frame_base64"].asString() == encoded,
         "snapshot returns the painted frame");

  Json::Value show_args;
  show_args["name"] = "splash";
  ToolResult shown = call_tool(host, "timebox_show", show_args);
  expect(!shown.is_error && shown.payload["focused_name"].asString() == "splash", "MCP show");
  ToolResult status = call_tool(host, "timebox_status", Json::Value(Json::objectValue));
  expect(!status.is_error && status.payload["process_count"].asInt() == 2, "show leaves both running");
  expect(status.payload["focused_name"].asString() == "splash", "status follows show");

  Json::Value bad;
  bad["process_id"] = paint.payload["id"].asInt();
  bad["frame_base64"] = "AAAA";
  ToolResult bad_frame = call_tool(host, "timebox_paint_frame", bad);
  expect(bad_frame.is_error, "short frame is a tool error");
}

void test_interrupt_timer_advances() {
  TimeboxHost host;
  host.start(/*present_loop=*/false, /*timer_loop=*/true);
  TimeboxProcess meeting;
  expect(host.start_process("timer", "live", 30, true, &meeting).empty(), "start live timer");
  const int initial = meeting.remaining_ms;
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  int remaining = initial;
  while (remaining == initial && std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    remaining = find_name(host.list_processes(), "live").remaining_ms;
  }
  expect(remaining < initial, "InterruptTimer advances the meeting");
  expect(remaining > initial - 5000, "a short live run does not skip the meeting");
  host.shutdown();
}

bool wait_frame(TimeboxHost &host, const std::vector<uint8_t> &want, int timeout_ms) {
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
  while (std::chrono::steady_clock::now() < deadline) {
    if (host.presented_frame() == want)
      return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  return host.presented_frame() == want;
}

void test_present_does_not_block() {
  TimeboxHost host;
  host.start(/*present_loop=*/true, /*timer_loop=*/false);

  expect(host.wait_present_count(2, 5000), "present runs with no processes and no MCP input");
  expect(host.status().sdl_ok, "headless SDL present is up");
  const std::vector<uint8_t> black(static_cast<size_t>(TimeboxHost::kFrameBytes), 0);
  expect(host.presented_frame() == black, "present holds a black frame before the first process");

  TimeboxProcess splash;
  expect(host.start_process("paint", "splash", -1, true, &splash).empty(), "start splash");
  std::vector<uint8_t> red(static_cast<size_t>(TimeboxHost::kFrameBytes), 0);
  for (size_t i = 0; i < red.size(); i += 3)
    red[i] = 200;

  host.set_present_stall_ms(500);
  bool proved = false;
  for (int attempt = 0; attempt < 6 && !proved; ++attempt) {
    if (!host.wait_until_present_busy(2000))
      break;
    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    // A short SDL draw also sets busy. The stall stays busy for 500 ms.
    if (!host.wait_until_present_busy(0))
      continue;
    const auto t0 = std::chrono::steady_clock::now();
    const std::string error = host.paint_frame(splash.id, red.data(), red.size(), nullptr);
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now() - t0)
                        .count();
    const bool still_busy = host.wait_until_present_busy(0);
    if (error.empty() && ms < 150 && still_busy)
      proved = true;
  }
  host.set_present_stall_ms(0);
  expect(proved, "paint_frame returns while present is stalled");

  expect(wait_frame(host, red, 5000), "present shows the painted frame");
  const uint64_t count = host.present_count();
  expect(host.wait_present_count(count + 2, 5000), "later presents are not skipped");
  expect(host.presented_frame() == red, "a late client holds the last frame");

  TimeboxProcess meeting;
  expect(host.start_process("timer", "meeting", 10, false, &meeting).empty(), "timer beside paint");
  expect(host.list_processes().size() == 2, "present host still has both processes");
  TimeboxProcess shown;
  expect(host.show_process(0, "meeting", &shown).empty(), "show meeting");
  const std::vector<uint8_t> face = timer_frame(10000, 10000);
  expect(wait_frame(host, face, 5000), "show changes the presented frame");
  std::vector<uint8_t> splash_frame;
  expect(host.snapshot(0, "splash", &splash_frame, nullptr).empty(), "splash snapshot");
  expect(splash_frame == red, "unfocused paint buffer survives show");

  expect(host.stop_process(meeting.id, "", nullptr).empty(), "stop focused timer");
  const uint64_t after_stop = host.present_count();
  expect(host.wait_present_count(after_stop + 2, 5000), "present continues after stop");
  expect(host.presented_frame() == face, "stopping the focus holds the last frame");
  expect(find_name(host.list_processes(), "splash").id == splash.id, "paint process still running");
  host.shutdown();
}

}  // namespace

int main() {
  test_two_processes_focus_and_paint();
  test_mcp_tools();
  test_interrupt_timer_advances();
  test_present_does_not_block();
  if (g_failures != 0) {
    std::cerr << g_failures << " timebox host checks failed\n";
    return 1;
  }
  std::cout << "PASS timebox host: two processes, focus, paint, nonblocking present\n";
  return 0;
}
