# Hardware

The product is a Raspberry Pi, a microphone, and a HUB75 LED matrix. The Bazel module name is `meetingbox`. The host you run on a development machine is `//src:timebox_mcp`, and on master that binary presents with SDL.

This page records facts already in the tree. Where a number has not been measured, it says so.

## What master runs

```bash
bazel run //src:timebox_mcp
```

That is the SDL / host demo. `TimeboxHost::present_loop` in `src/timebox/host.cpp` constructs `graphics::SDL2Renderer` and calls `init_capture()`. If `SDL_VIDEODRIVER` is unset, the loop sets it to `dummy`. Logs go to stderr. JSON-RPC stays on stdout. A late frame leaves the previous pixels in place.

The select that presents those frames on the HUB75 matrix is not on master. It is open pull request [#7](https://github.com/danielcloran/meetingbox-cpp/pull/7) (`cursor/hub75-present-select-a2fd`). That pull request keeps `//src:timebox_mcp` as the host and is unmerged. This document does not add a `--config=pi` flag and does not add an aarch64 CI job.

Until #7 is on master, `//src:timebox_mcp` does not compile `media/src/graphics/rpi/renderer.cpp`. That file is the in-tree HUB75 driver. It is not a `srcs` entry on master (`media/BUILD.bazel` builds `graphics.cpp` for the graphics library). The mapper header sits in the `include/graphics/**/*.hpp` glob, and no file master compiles includes it.

## Strip layout

A frame is packed RGB888, row-major, no padding:

- 320×64
- 61440 bytes (`TimeboxHost::kFrameBytes` in `include/timebox/host.hpp`)
- pixel `(x, y)` starts at byte `(y * 320 + x) * 3`, then R, G, B (`draw_rgb` in `src/timebox/host.cpp`)

Five 64×64 panels sit side by side. Strip index 2 is the lid. MCP describes the same order (`timebox_paint_frame` in `src/timebox/mcp.cpp`).

| Strip index | x origin | Face | Role |
| --- | --- | --- | --- |
| 0 | 0 | Left | wall |
| 1 | 64 | Front | wall |
| 2 | 128 | Top | lid |
| 3 | 192 | Right | wall |
| 4 | 256 | Back | wall |

`paint_box_layout` in `media/src/graphics/box_layout.cpp` unfolds that strip into a 192×192 cross. The lid is the center cell, the face you look down on. Index 4 is the back wall. An earlier graphics split treated index 4 as the top; that reading is wrong, and the comment in `box_layout.cpp` says so.

```text
              Back (4)
  Left (0)   Top (2)   Right (3)
              Front (1)
```

Destinations on the 192×192 buffer, each cell 64×64 (`kPlacements` in `box_layout.cpp`):

| Face | Strip index | Cross origin |
| --- | --- | --- |
| Back | 4 | (64, 0) |
| Left | 0 | (0, 64) |
| Top / lid | 2 | (64, 64) |
| Right | 3 | (128, 64) |
| Front | 1 | (64, 128) |

The four corners stay the background colour `rgb(16, 16, 16)` (`kBoxBackground` in `media/include/graphics/graphics.hpp`).

`paint_graphics_test_pattern` fills the strip with a labelled solid colour per face: Left red, Front green, Top blue, Right yellow, Back magenta. On the SDL host, check the cross with:

```bash
bazel test //media:box_layout_test
bazel run //media:graphics_test -- --headless --ppm=/tmp/box.ppm
```

Those commands read the software renderer back. They do not light the panels.

## HUB75 driver options

The product panels are HUB75, FM6126A, via [rpi-rgb-led-matrix](https://github.com/hzeller/rpi-rgb-led-matrix). They are not a WS281x string.

`media/src/graphics/rpi/renderer.cpp` on master sets:

| Option | Value |
| --- | --- |
| `hardware_mapping` | `regular` |
| `panel_type` | `FM6126A` |
| `rows`, `cols` | 64, 64 |
| `chain_length` | 2 |
| `parallel` | 3 |
| `led_rgb_sequence` | `BGR` |
| `pwm_lsb_nanoseconds` | 50 |
| `gpio_slowdown` | 4 (`RuntimeOptions`) |
| `show_refresh_rate` | `true` in this unlinked file |

`BGR` is the library's panel colour order. The host frame stays packed RGB888. This page does not restate the GPIO table behind `hardware_mapping = "regular"`; that table lives in rpi-rgb-led-matrix, and this repo does not copy it.

Pull request #7 keeps the same options except `show_refresh_rate`, which it sets to `false`. The library prints that counter on stdout, and the MCP server owns stdout. #7 fetches `hzeller/rpi-rgb-led-matrix` at commit `3da7a3bdfca4637bb79ff038bc2bb26965197db5` and selects the HUB75 renderer for the Pi present path. None of that select is on master.

### Chain map

`chain_length` 2 and `parallel` 3 with 64×64 panels give a 128×192 address space (2×64 by 3×64). `CustomMapper` in `media/include/graphics/rpi/pixel_mapper.hpp` exposes a 320×64 visible canvas and tiles the strip onto that grid. It does not name cube faces and it does not build the cross above. Mapper comments number panels 1..5 in strip order, so mapper panel 3 is the lid (strip index 2). Mapper panel 2 is the Front wall.

The function body does this:

| Visible x | Mapper name | Face (strip index) | Matrix origin from the arithmetic |
| --- | --- | --- | --- |
| 0–63 | panel 1 | Left (0) | (0, 0) |
| 64–127 | panel 2 | Front (1) | (64, 0) |
| 128–191 | panel 3 | Top / lid (2) | (0, 64) |
| 192–255 | panel 4 | Right (3) | (64, 64) |
| 256–319 | panel 5 | Back (4) | (64, 128) |

Panel 5 is `matrix_x = visible_x - 192`, `matrix_y = visible_y + 128`. The comment above that assignment says `256, 0 -> 128, 64`. That comment does not match the arithmetic. Trust the arithmetic: visible x 256 lands at matrix (64, 128). Pull request #7 keeps the same comment and the same arithmetic.

```text
matrix x:      0          64
y 0:           Left       Front
y 64:          Top (lid)  Right
y 128:         (no panel)   Back
```

Nothing in the mapper writes the left cell of the third row.

## Do not use WS281x

The panels are HUB75. The `@rpi_ws281x` path is the wrong hardware and must not be used.

On master it is still wired:

- `MODULE.bazel` fetches `jgarff/rpi_ws281x` (commit `7fc8914ba08f862351ab64c810c9d31190cf6c1a`) and comments it as WS2812B LED control.
- `//media:graphics` selects `@rpi_ws281x//:ws2811` and `-DRASPBERRY_PI` when `//media:raspberry_pi` matches (`@platforms//os:linux` and `@platforms//cpu:aarch64`) in `media/BUILD.bazel`.
- `graphics::initialize` in `media/src/graphics/graphics.cpp` then constructs `RpiRenderer`, which calls `ws2811_init`.

That init is a single WS2811 string, not five HUB75 panels:

- GPIO 18 (PWM0), `output.channel[0].gpionum = 18`
- `count = LED_WIDTH * LED_HEIGHT` (320×64 = 20480)
- `strip_type = WS2811_STRIP_GRB`
- DMA channel 10, brightness 255

The comment next to that count says 20480 LEDs is huge for one channel and guesses that a HAT will remap them. That guess is the wrong product. `//src:meetingbox` calls `graphics::initialize()`, so an aarch64 build of that binary follows this path. Do not treat that as the matrix driver.

`//src:timebox_mcp` on master does not call `ws2811_init`. It also does not call the HUB75 driver. Pull request #7 removes the `@rpi_ws281x` fetch and points the Pi present path at rpi-rgb-led-matrix. Until that is merged, leave the WS281x code unused.

## Microphone

A microphone is part of the product. This repository does not record the microphone model, the connector, or the capture device used on the Pi. `//src:timebox_mcp` does not start audio. PocketSphinx is linked by the separate `//media:audio` library, which is not the present path.

## Power

No current measurement for this box is in the repo. TODO: measure.

Before a long run, measure the 5V rail at the panels. A full-white frame is a brief check, not a soak. Do not copy an amp figure into this document until it has been measured here. This tree does not record a PSU model or a power-connector pinout.
