#include "timebox/mcp.hpp"

#include "json/json.h"

#include <initializer_list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr const char *kProtocol = "2024-11-05";

std::string dump(const Json::Value &value) {
  Json::StreamWriterBuilder builder;
  builder["indentation"] = "";
  builder["commentStyle"] = "None";
  std::string text = Json::writeString(builder, value);
  while (!text.empty() && (text.back() == '\n' || text.back() == '\r'))
    text.pop_back();
  return text;
}

bool parse_json(const std::string &line, Json::Value &root, std::string &error) {
  Json::CharReaderBuilder builder;
  builder["collectComments"] = false;
  const std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
  return reader->parse(line.data(), line.data() + line.size(), &root, &error);
}

Json::Value rpc_error(const Json::Value &id, int code, const std::string &message) {
  Json::Value value;
  value["jsonrpc"] = "2.0";
  value["id"] = id;
  value["error"]["code"] = code;
  value["error"]["message"] = message;
  return value;
}

Json::Value tool_text(const Json::Value &id, const std::string &text, bool is_error) {
  Json::Value value;
  value["jsonrpc"] = "2.0";
  value["id"] = id;
  value["result"]["content"][0]["type"] = "text";
  value["result"]["content"][0]["text"] = text;
  value["result"]["isError"] = is_error;
  return value;
}

Json::Value tool_ok(const Json::Value &id, const Json::Value &payload) {
  return tool_text(id, dump(payload), false);
}

Json::Value tool_fail(const Json::Value &id, const std::string &message) {
  return tool_text(id, message, true);
}

int b64_value(char c) {
  if (c >= 'A' && c <= 'Z')
    return c - 'A';
  if (c >= 'a' && c <= 'z')
    return c - 'a' + 26;
  if (c >= '0' && c <= '9')
    return c - '0' + 52;
  if (c == '+')
    return 62;
  if (c == '/')
    return 63;
  return -1;
}

bool base64_decode(const std::string &in, std::vector<uint8_t> &out, std::string &error) {
  std::string text;
  text.reserve(in.size());
  for (char c : in) {
    if (c == '=' || b64_value(c) >= 0)
      text.push_back(c);
    else if (c == '\n' || c == '\r' || c == ' ')
      continue;
    else {
      error = "bad base64";
      return false;
    }
  }
  if (text.size() % 4 != 0) {
    error = "bad base64";
    return false;
  }
  out.clear();
  out.reserve(text.size() / 4 * 3);
  for (size_t i = 0; i < text.size(); i += 4) {
    int n[4] = {0, 0, 0, 0};
    int pad = 0;
    for (int j = 0; j < 4; ++j) {
      if (text[i + j] == '=') {
        ++pad;
      } else {
        n[j] = b64_value(text[i + j]);
        if (n[j] < 0 || pad > 0) {
          error = "bad base64";
          return false;
        }
      }
    }
    out.push_back(static_cast<uint8_t>((n[0] << 2) | (n[1] >> 4)));
    if (pad < 2)
      out.push_back(static_cast<uint8_t>(((n[1] & 15) << 4) | (n[2] >> 2)));
    if (pad < 1)
      out.push_back(static_cast<uint8_t>(((n[2] & 3) << 6) | n[3]));
  }
  return true;
}

std::string base64_encode(const uint8_t *data, size_t n) {
  static const char *table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string out;
  out.reserve((n + 2) / 3 * 4);
  size_t i = 0;
  for (; i + 3 <= n; i += 3) {
    const unsigned value = (static_cast<unsigned>(data[i]) << 16) |
                           (static_cast<unsigned>(data[i + 1]) << 8) |
                           static_cast<unsigned>(data[i + 2]);
    out.push_back(table[(value >> 18) & 63]);
    out.push_back(table[(value >> 12) & 63]);
    out.push_back(table[(value >> 6) & 63]);
    out.push_back(table[value & 63]);
  }
  if (n - i == 1) {
    const unsigned value = static_cast<unsigned>(data[i]) << 16;
    out.push_back(table[(value >> 18) & 63]);
    out.push_back(table[(value >> 12) & 63]);
    out.push_back('=');
    out.push_back('=');
  } else if (n - i == 2) {
    const unsigned value = (static_cast<unsigned>(data[i]) << 16) |
                           (static_cast<unsigned>(data[i + 1]) << 8);
    out.push_back(table[(value >> 18) & 63]);
    out.push_back(table[(value >> 12) & 63]);
    out.push_back(table[(value >> 6) & 63]);
    out.push_back('=');
  }
  return out;
}

Json::Value process_json(const TimeboxProcess &process) {
  Json::Value value;
  value["id"] = process.id;
  value["name"] = process.name;
  value["kind"] = process.kind;
  value["focused"] = process.focused;
  value["seconds"] = process.seconds;
  value["remaining_ms"] = process.remaining_ms;
  value["frame_seq"] = static_cast<Json::UInt64>(process.frame_seq);
  value["done"] = process.done;
  return value;
}

Json::Value prop(const char *type, const char *description) {
  Json::Value value;
  value["type"] = type;
  if (description)
    value["description"] = description;
  return value;
}

Json::Value schema(Json::Value properties, std::initializer_list<const char *> required) {
  Json::Value value;
  value["type"] = "object";
  value["properties"] = std::move(properties);
  value["required"] = Json::Value(Json::arrayValue);
  for (const char *name : required)
    value["required"].append(name);
  return value;
}

Json::Value tool_list() {
  Json::Value tools(Json::arrayValue);

  auto add = [&](const char *name, const char *description, Json::Value input) {
    Json::Value tool;
    tool["name"] = name;
    tool["description"] = description;
    tool["inputSchema"] = std::move(input);
    tools.append(std::move(tool));
  };

  add("timebox_status",
      "Report which process owns the LEDs, how many are running, and how many frames the present thread has shown.",
      schema(Json::Value(Json::objectValue), {}));

  add("timebox_list_processes",
      "List running processes. Finished timers are removed. Unfocused processes keep running until they finish or are stopped.",
      schema(Json::Value(Json::objectValue), {}));

  Json::Value start_props;
  start_props["kind"] = prop("string", "timer or paint");
  start_props["kind"]["enum"] = Json::Value(Json::arrayValue);
  start_props["kind"]["enum"].append("timer");
  start_props["kind"]["enum"].append("paint");
  start_props["name"] = prop("string", "Unique process name");
  start_props["seconds"] = prop("integer", "Timer length. Default 60. Ignored for paint.");
  start_props["focus"] = prop("boolean", "Show this process on the LEDs. Default true. Does not stop others.");
  add("timebox_start",
      "Start a timer or paint process. At most 8 run at once. Finished timers are dropped first so their slots can be reused; eight live processes still fail. focus defaults to true and does not stop other processes.",
      schema(std::move(start_props), {"kind", "name"}));

  Json::Value id_name;
  id_name["id"] = prop("integer", "Process id");
  id_name["name"] = prop("string", "Process name");
  add("timebox_stop",
      "Stop one process by id or name. Others keep running. Use this for a live process; a finished timer is already gone.",
      schema(id_name, {}));

  Json::Value show_props = id_name;
  add("timebox_show",
      "Focus the LEDs on a process by id or name. Does not stop the others.",
      schema(std::move(show_props), {}));

  Json::Value paint_props;
  paint_props["process_id"] = prop("integer", "Paint process id");
  paint_props["frame_base64"] = prop(
      "string", "Packed RGB888, 320x64, panels Left|Front|Top|Right|Back. Lid is panel 2.");
  add("timebox_paint_frame",
      "Store one RGB888 frame on a paint process. Works while that process is unfocused.",
      schema(std::move(paint_props), {"process_id", "frame_base64"}));

  add("timebox_snapshot",
      "Return the latest RGB888 frame for the focused process, or the one named by id or name.",
      schema(id_name, {}));

  Json::Value result;
  result["tools"] = std::move(tools);
  return result;
}

int arg_id(const Json::Value &args) {
  if (args.isMember("process_id") && args["process_id"].isInt())
    return args["process_id"].asInt();
  if (args.isMember("id") && args["id"].isInt())
    return args["id"].asInt();
  return 0;
}

std::string arg_name(const Json::Value &args) {
  if (args.isMember("name") && args["name"].isString())
    return args["name"].asString();
  return {};
}

Json::Value call_tool(TimeboxHost &host, const Json::Value &id, const Json::Value &params) {
  if (!params.isMember("name") || !params["name"].isString())
    return tool_fail(id, "tools/call requires a name");
  const std::string name = params["name"].asString();
  Json::Value args(Json::objectValue);
  if (params.isMember("arguments") && params["arguments"].isObject())
    args = params["arguments"];

  if (name == "timebox_status") {
    const TimeboxStatus status = host.status();
    Json::Value payload;
    payload["focused_id"] = status.focused_id;
    payload["focused_name"] = status.focused_name;
    payload["process_count"] = status.process_count;
    payload["capacity"] = status.capacity;
    payload["present_count"] = static_cast<Json::UInt64>(status.present_count);
    payload["present_running"] = status.present_running;
    return tool_ok(id, payload);
  }

  if (name == "timebox_list_processes") {
    Json::Value payload;
    payload["processes"] = Json::Value(Json::arrayValue);
    for (const auto &process : host.list_processes())
      payload["processes"].append(process_json(process));
    return tool_ok(id, payload);
  }

  if (name == "timebox_start") {
    if (!args.isMember("kind") || !args["kind"].isString())
      return tool_fail(id, "kind must be timer or paint");
    if (!args.isMember("name") || !args["name"].isString())
      return tool_fail(id, "name is required");
    int seconds = -1;
    if (args.isMember("seconds") && !args["seconds"].isNull()) {
      if (!args["seconds"].isInt())
        return tool_fail(id, "seconds must be an integer");
      seconds = args["seconds"].asInt();
    }
    const bool focus = args.isMember("focus") ? args["focus"].asBool() : true;
    TimeboxProcess created;
    const std::string error =
        host.start_process(args["kind"].asString(), args["name"].asString(), seconds, focus, &created);
    if (!error.empty())
      return tool_fail(id, error);
    return tool_ok(id, process_json(created));
  }

  if (name == "timebox_stop") {
    TimeboxProcess stopped;
    const std::string error = host.stop_process(arg_id(args), arg_name(args), &stopped);
    if (!error.empty())
      return tool_fail(id, error);
    Json::Value payload = process_json(stopped);
    payload["stopped_id"] = stopped.id;
    return tool_ok(id, payload);
  }

  if (name == "timebox_show") {
    TimeboxProcess focused;
    const std::string error = host.show_process(arg_id(args), arg_name(args), &focused);
    if (!error.empty())
      return tool_fail(id, error);
    Json::Value payload;
    payload["focused_id"] = focused.id;
    payload["focused_name"] = focused.name;
    payload["kind"] = focused.kind;
    return tool_ok(id, payload);
  }

  if (name == "timebox_paint_frame") {
    const int id_arg = args.isMember("process_id") && args["process_id"].isInt()
                           ? args["process_id"].asInt()
                           : 0;
    if (id_arg == 0)
      return tool_fail(id, "process_id is required");
    if (!args.isMember("frame_base64") || !args["frame_base64"].isString())
      return tool_fail(id, "frame_base64 is required");
    std::vector<uint8_t> rgb;
    std::string error;
    if (!base64_decode(args["frame_base64"].asString(), rgb, error))
      return tool_fail(id, error);
    TimeboxProcess updated;
    error = host.paint_frame(id_arg, rgb.data(), rgb.size(), &updated);
    if (!error.empty())
      return tool_fail(id, error);
    Json::Value payload;
    payload["process_id"] = updated.id;
    payload["frame_seq"] = static_cast<Json::UInt64>(updated.frame_seq);
    payload["focused"] = updated.focused;
    return tool_ok(id, payload);
  }

  if (name == "timebox_snapshot") {
    std::vector<uint8_t> frame;
    TimeboxProcess info;
    const std::string error = host.snapshot(arg_id(args), arg_name(args), &frame, &info);
    if (!error.empty())
      return tool_fail(id, error);
    Json::Value payload = process_json(info);
    payload["width"] = TimeboxHost::kWidth;
    payload["height"] = TimeboxHost::kHeight;
    payload["encoding"] = "rgb888";
    payload["frame_base64"] = base64_encode(frame.data(), frame.size());
    return tool_ok(id, payload);
  }

  return tool_fail(id, "unknown tool: " + name);
}

}  // namespace

std::string handle_mcp_message(TimeboxHost &host, const std::string &line) {
  std::string text = line;
  while (!text.empty() && (text.back() == '\n' || text.back() == '\r' || text.back() == ' '))
    text.pop_back();
  if (text.empty())
    return {};

  Json::Value root;
  std::string error;
  if (!parse_json(text, root, error) || !root.isObject())
    return dump(rpc_error(Json::Value(), -32700, "parse error"));

  const bool notification = !root.isMember("id") || root["id"].isNull();
  const Json::Value id = notification ? Json::Value() : root["id"];
  if (!root.isMember("method") || !root["method"].isString()) {
    if (notification)
      return {};
    return dump(rpc_error(id, -32600, "invalid request"));
  }
  const std::string method = root["method"].asString();
  Json::Value params(Json::objectValue);
  if (root.isMember("params") && root["params"].isObject())
    params = root["params"];

  if (method == "notifications/initialized" || method == "initialized" ||
      method.rfind("notifications/", 0) == 0) {
    if (notification)
      return {};
    Json::Value response;
    response["jsonrpc"] = "2.0";
    response["id"] = id;
    response["result"] = Json::Value(Json::objectValue);
    return dump(response);
  }

  if (method == "initialize") {
    Json::Value result;
    std::string version = kProtocol;
    if (params.isMember("protocolVersion") && params["protocolVersion"].isString() &&
        !params["protocolVersion"].asString().empty()) {
      version = params["protocolVersion"].asString();
    }
    result["protocolVersion"] = version;
    result["capabilities"]["tools"]["listChanged"] = false;
    result["serverInfo"]["name"] = "timebox";
    result["serverInfo"]["version"] = "0.1.0";
    Json::Value response;
    response["jsonrpc"] = "2.0";
    response["id"] = id;
    response["result"] = std::move(result);
    return dump(response);
  }

  if (method == "ping") {
    Json::Value response;
    response["jsonrpc"] = "2.0";
    response["id"] = id;
    response["result"] = Json::Value(Json::objectValue);
    return dump(response);
  }

  if (method == "tools/list") {
    Json::Value response;
    response["jsonrpc"] = "2.0";
    response["id"] = id;
    response["result"] = tool_list();
    return dump(response);
  }

  if (method == "tools/call")
    return dump(call_tool(host, id, params));

  if (notification)
    return {};
  return dump(rpc_error(id, -32601, "method not found"));
}
