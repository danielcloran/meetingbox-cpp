#include "graphics/graphics.hpp"
#include "graphics/timebox_timer.hpp"
#include "processes/timer.hpp"
#include "utils/interrupt_timer.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

#include "json/json.h"

namespace {

int g_failures = 0;

void fail(const std::string &message) {
  std::cerr << "FAIL " << message << "\n";
  ++g_failures;
}

void expect(bool cond, const std::string &message) {
  if (!cond)
    fail(message);
}

graphics::Pixel at(const graphics::GraphicsContext &image, int x, int y) {
  return image.get_pixels()[y * image.get_width() + x];
}

int count_diff(const graphics::GraphicsContext &a, const graphics::GraphicsContext &b) {
  const auto &pa = a.get_pixels();
  const auto &pb = b.get_pixels();
  int diff = 0;
  const int n = std::min(static_cast<int>(pa.size()), static_cast<int>(pb.size()));
  for (int i = 0; i < n; ++i) {
    if (pa[i] != pb[i])
      ++diff;
  }
  return diff;
}

bool box_for_strip(int strip_x, int strip_y, int &box_x, int &box_y) {
  const int panel = strip_x / graphics::kPanelSize;
  const int local_x = strip_x % graphics::kPanelSize;
  const graphics::BoxPanelPlacement *placements = graphics::box_panel_placements();
  for (int i = 0; i < graphics::kBoxPanelCount; ++i) {
    if (placements[i].strip_index != panel)
      continue;
    box_x = placements[i].dest_x + local_x;
    box_y = placements[i].dest_y + strip_y;
    return true;
  }
  return false;
}

void check_formula() {
  expect(graphics::timebox_row_blue(0, 64, 1.f) == 255, "full timer top row is blue");
  expect(graphics::timebox_row_blue(63, 64, 1.f) == 255, "full timer bottom row is blue");
  expect(graphics::timebox_row_blue(0, 64, 0.5f) == 0, "half timer top row is off");
  expect(graphics::timebox_row_blue(32, 64, 0.5f) == 0, "half timer midpoint row is off");
  expect(graphics::timebox_row_blue(63, 64, 0.5f) == 125, "half timer bottom row is mid blue");
  expect(graphics::timebox_row_blue(60, 64, 0.5f) == 118, "half timer lower row fades");
  expect(graphics::timebox_row_blue(48, 64, 0.75f) == 169, "three-quarter row matches the fade");
  expect(graphics::timebox_row_blue(63, 64, 0.f) == 0, "empty timer is black");
  expect(graphics::timebox_row_blue(10, 64, 0.5f) < graphics::timebox_row_blue(60, 64, 0.5f),
         "the fade is brighter toward the bottom");
}

void check_strip_and_box() {
  graphics::GraphicsContext full(LED_WIDTH, LED_HEIGHT);
  graphics::GraphicsContext half(LED_WIDTH, LED_HEIGHT);
  graphics::GraphicsContext empty(LED_WIDTH, LED_HEIGHT);
  graphics::paint_timebox_timer(full, 10000, 10000);
  graphics::paint_timebox_timer(half, 5000, 10000);
  graphics::paint_timebox_timer(empty, 0, 10000);

  const graphics::Pixel blue{0, 0, 255};
  const graphics::Pixel off{0, 0, 0};
  const graphics::Pixel mid{0, 0, 125};
  // Side faces (panels 0-3) share the row-fade. x=300 is on the lid.
  expect(at(full, 10, 0) == blue && at(full, 200, 63) == blue,
         "a full countdown fills the side faces");
  expect(at(half, 10, 0) == off, "half countdown clears the side top row");
  expect(at(half, 10, 63) == mid, "half countdown leaves the side bottom row mid blue");
  expect(at(empty, 10, 63) == off, "a finished side face is black");
  for (int panel = 0; panel < graphics::kTimeboxTopPanel; ++panel) {
    const int x = panel * graphics::kPanelSize + 8;
    expect(at(half, x, 63) == mid, "every side face shows the same countdown");
    expect(at(half, x, 63) != at(half, graphics::kTimeboxTopPanel * graphics::kPanelSize + 8, 63),
           "the lid is not a copy of the side countdown");
  }
  const int lid = graphics::kTimeboxTopPanel * graphics::kPanelSize;
  const int bar_y = graphics::kTimeboxLidBarY + graphics::kTimeboxLidBarH / 2;
  const int bar_left = lid + graphics::kTimeboxLidBarInset;
  const int bar_far = bar_left + 40;
  expect(at(full, bar_left, bar_y) == graphics::kTimeboxLidFill, "a full lid bar is filled");
  expect(at(full, bar_far, bar_y) == graphics::kTimeboxLidFill, "a full lid bar reaches the far end");
  expect(at(half, bar_left, bar_y) == graphics::kTimeboxLidFill, "half remaining still lights the start of the bar");
  expect(at(half, bar_far, bar_y) == graphics::kTimeboxLidTrack, "half remaining leaves the far end of the bar dark");
  expect(at(empty, bar_left, bar_y) == graphics::kTimeboxLidTrack, "a finished lid bar is only the track");
  const int lamp_x = lid + (graphics::kPanelSize - graphics::kTimeboxLidLamp) / 2 + 2;
  const int lamp_y = graphics::kTimeboxLidLampY + 2;
  expect(at(full, lamp_x, lamp_y) == graphics::kTimeboxLidInMeeting, "the lid lamp is in-meeting green");
  expect(at(half, lamp_x, lamp_y) == graphics::kTimeboxLidInMeeting, "halfway the lid lamp is still in meeting");
  expect(at(empty, lamp_x, lamp_y) == graphics::kTimeboxLidOver, "a finished meeting turns the lid lamp red");
  graphics::GraphicsContext ending(LED_WIDTH, LED_HEIGHT);
  graphics::paint_timebox_timer(ending, 1000, 10000);
  expect(at(ending, lamp_x, lamp_y) == graphics::kTimeboxLidEnding,
         "the last quarter turns the lid lamp amber");
  expect(at(full, bar_left, bar_y) != at(full, 10, bar_y),
         "the lid bar is not the side-face blue");
  expect(count_diff(full, half) > 1000, "full and half frames differ across the strip");
  expect(count_diff(half, empty) > 1000, "half and empty frames differ across the strip");

  graphics::GraphicsContext full_box(graphics::kBoxWidth, graphics::kBoxHeight);
  graphics::GraphicsContext half_box(graphics::kBoxWidth, graphics::kBoxHeight);
  graphics::paint_box_layout(full, full_box);
  graphics::paint_box_layout(half, half_box);

  const int face_rows[] = {0, 63};
  const int panels[] = {0, 1, 2, 3, graphics::kTimeboxTopPanel};
  for (int panel : panels) {
    for (int row : face_rows) {
      int box_x = 0;
      int box_y = 0;
      const int strip_x = panel * graphics::kPanelSize + 8;
      expect(box_for_strip(strip_x, row, box_x, box_y), "panel maps onto the cross");
      expect(at(full_box, box_x, box_y) == at(full, strip_x, row),
             "full face matches the strip");
      expect(at(half_box, box_x, box_y) == at(half, strip_x, row),
             "half face matches the strip");
    }
  }
  expect(at(half_box, 0, 0) == graphics::kBoxBackground, "corner stays the box background");
  expect(at(half_box, 160, 160) == graphics::kBoxBackground, "opposite corner stays empty");
  int lid_box_x = 0;
  int lid_box_y = 0;
  int side_box_x = 0;
  int side_box_y = 0;
  expect(box_for_strip(bar_far, bar_y, lid_box_x, lid_box_y), "lid bar maps onto the cross");
  expect(box_for_strip(10, bar_y, side_box_x, side_box_y), "side face maps onto the cross");
  expect(at(half_box, lid_box_x, lid_box_y) == graphics::kTimeboxLidTrack,
         "the unfolded lid shows the unfilled end of the status bar");
  expect(at(half_box, side_box_x, side_box_y) != at(half_box, lid_box_x, lid_box_y),
         "on the box, the lid is not the same pixels as a side face");
  expect(count_diff(full_box, half_box) > 1000, "the unfolded box changes as the timer drops");

  setenv("SDL_VIDEODRIVER", "dummy", 1);
  graphics::GraphicsContext captured_full(graphics::kBoxWidth, graphics::kBoxHeight);
  graphics::GraphicsContext captured_half(graphics::kBoxWidth, graphics::kBoxHeight);
  std::string error;
  if (!graphics::capture_painted_box(full, captured_full, error))
    fail("SDL capture (full): " + error);
  else if (count_diff(captured_full, full_box) != 0)
    fail("SDL readback does not match the full countdown layout");
  if (!graphics::capture_painted_box(half, captured_half, error))
    fail("SDL capture (half): " + error);
  else if (count_diff(captured_half, half_box) != 0)
    fail("SDL readback does not match the half countdown layout");
  else if (count_diff(captured_full, captured_half) < 1000)
    fail("SDL frames of the countdown do not change over time");
  else
    std::cout << "PASS SDL readback of two countdown frames\n";
}

void check_timer_process() {
  graphics::context_ =
      std::make_unique<graphics::GraphicsContext>(LED_WIDTH, LED_HEIGHT);
  Json::Value info;
  info["time"] = 10;
  TimerProcess timer(1, 0, info);
  expect(timer.total_ms() == 10000, "integer seconds become milliseconds");
  expect(timer.remaining_ms() == 10000, "countdown starts full");

  InterruptTimer clock;
  clock.tick(0);
  events::queue.process();
  expect(timer.remaining_ms() == 10000, "a zero tick does not consume the meeting");
  expect(at(*graphics::context_, 4, 63) == graphics::Pixel(0, 0, 255),
         "first paint is a full blue field");

  const auto frame_full = graphics::context_->get_pixels();
  clock.tick(5000);
  events::queue.process();
  expect(timer.remaining_ms() == 5000, "InterruptTimer tick subtracts elapsed ms");
  expect(at(*graphics::context_, 4, 0) == graphics::Pixel(0, 0, 0),
         "after half the meeting the top row is off");
  expect(at(*graphics::context_, 4, 63) == graphics::Pixel(0, 0, 125),
         "after half the meeting the bottom row is mid blue");
  expect(frame_full != graphics::context_->get_pixels(),
         "pixels change after the timer advances");

  clock.tick(5000);
  events::queue.process();
  expect(timer.remaining_ms() == 0, "the meeting reaches zero");
  expect(at(*graphics::context_, 4, 63) == graphics::Pixel(0, 0, 0),
         "a finished meeting paints black");

  clock.tick(1000);
  events::queue.process();
  expect(timer.remaining_ms() == 0, "a deleted timer does not run negative");
}

void check_duration_parse() {
  Json::Value seconds;
  seconds["time"] = "PT5S";
  TimerProcess five(2, 0, seconds);
  expect(five.total_ms() == 5000, "PT5S is 5 seconds");

  Json::Value minute;
  minute["time"] = "PT1M";
  TimerProcess one_minute(3, 0, minute);
  expect(one_minute.total_ms() == 60000, "PT1M is 60 seconds");

  Json::Value text;
  text["time"] = "15";
  TimerProcess fifteen(4, 0, text);
  expect(fifteen.total_ms() == 15000, "numeric text is seconds");
}

void check_live_clock() {
  graphics::context_ =
      std::make_unique<graphics::GraphicsContext>(LED_WIDTH, LED_HEIGHT);
  Json::Value info;
  info["time"] = 30;
  TimerProcess timer(5, 0, info);
  const int total = timer.total_ms();
  InterruptTimer clock;
  clock.setFrameRate(20);
  clock.tick(0);
  events::queue.process();
  const auto before = graphics::context_->get_pixels();
  std::thread ticker([&clock] { clock.update(); });
  const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (timer.remaining_ms() == total &&
         std::chrono::steady_clock::now() < deadline) {
    events::queue.process();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
  events::queue.enqueue(std::make_shared<StopEvent>());
  events::queue.process();
  if (ticker.joinable())
    ticker.join();
  events::queue.process();

  const int remaining = timer.remaining_ms();
  expect(remaining < total, "InterruptTimer::update advances the meeting");
  expect(remaining > total - 5000, "a short live run does not skip the meeting");
  expect(graphics::context_->get_pixels() != before,
         "live ticks change the painted faces");
}

}  // namespace

int main() {
  check_formula();
  check_strip_and_box();
  check_timer_process();
  check_duration_parse();
  check_live_clock();
  if (g_failures != 0) {
    std::cerr << g_failures << " timer paint checks failed\n";
    return 1;
  }
  std::cout << "PASS TimeBox countdown advances and the painted faces change\n";
  return 0;
}
