# TimeBox MCP

TimeBox / MeetingBox targets a Raspberry Pi with a microphone and a physical HUB75 LED matrix. The panels are FM6126A 64×64 modules, chain length 2 and parallel 3, driven by [rpi-rgb-led-matrix](https://github.com/hzeller/rpi-rgb-led-matrix) (`hardware_mapping` `regular`, `gpio_slowdown` 4, LED order `BGR`). SDL is the demo and the default host only. The strip, the chain map, and a power note are in [docs/hardware.md](docs/hardware.md).

The Pi present select is not on master. It is open pull request [#7](https://github.com/danielcloran/meetingbox-cpp/pull/7). Until that lands, the command below is the SDL host.

**Do not use `@rpi_ws281x` or the GPIO 18 `ws2811` path.** That driver targets a WS2811/WS2812B string. These panels are HUB75. Master still contains that code. See [docs/hardware.md](docs/hardware.md#do-not-use-ws281x).

No panel current is recorded in this repo. Measure the 5V rail, and keep a full-white frame brief. TODO: measure. See [docs/hardware.md](docs/hardware.md#power).

The box shows one process. Others keep running until `timebox_stop`.
Clients push frames. The present thread copies the focused 320×64 slot and never reads MCP input. If a new frame is late, the last one stays on the LEDs.

A frame is packed RGB888, 320×64 (61440 bytes), five 64×64 panels: `Left | Front | Top | Right | Back`. The lid is panel index 2, the center of the unfolded cross.

`timer` uses `InterruptTimer` and the existing faces (blue row-fade on the walls, spiral on the lid). `paint` only stores the latest frame a client pushed. At most 8 processes.

Audio, HTTP, and a skill registry are not part of this host. The product includes a microphone; this host does not start it, and the microphone model is not recorded in the tree.

## Run the server

```bash
bazel run //src:timebox_mcp
```

Newline-delimited JSON-RPC on stdin, responses on stdout, logs on stderr. On master the present thread is headless SDL (`SDL_VIDEODRIVER=dummy` when that variable is unset). It does not open the HUB75 driver and it does not open `ws2811`.

## Tools

- `timebox_status` — focus, process count, frames the present thread has shown
- `timebox_list_processes`
- `timebox_start` — `kind` (`timer` or `paint`), `name`, optional `seconds` (timer default 60), optional `focus` (default true). Focusing does not stop anyone else.
- `timebox_stop` — `id` or `name`
- `timebox_show` — `id` or `name`. Switches the LEDs only.
- `timebox_paint_frame` — `process_id` and `frame_base64` (raw RGB888 bytes). Works while that process is unfocused.
- `timebox_snapshot` — focused process, or pass `id` / `name`

## Two-process demo

Starts a 30s meeting timer and a paint process, pushes a green lid, and `show`s each in turn.

```bash
bazel build //src:timebox_mcp
python3 examples/two_process_demo.py bazel-bin/src/timebox_mcp
```

## Test

```bash
bazel test //src:timebox_host_test
```

The unfolded cross (SDL, not the panels) is checked by `bazel test //media:box_layout_test`.
