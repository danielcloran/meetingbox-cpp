# TimeBox MCP

The box shows one process. Others keep running until `timebox_stop`.
Clients push frames. The present thread copies the focused 320×64 slot and never reads MCP input. If a new frame is late, the last one stays on the LEDs.

A frame is packed RGB888, 320×64, five 64×64 panels: `Left | Front | Top | Right | Back`. The lid is the top panel (index 2).

`timer` uses `InterruptTimer` and the existing faces (blue row-fade on the walls, spiral on the lid). `paint` only stores the latest frame a client pushed. At most 8 processes.

Audio, HTTP, and a skill registry are not part of this host.

## Run the server

```bash
bazel run //src:timebox_mcp
```

Newline-delimited JSON-RPC on stdin, responses on stdout, logs on stderr. SDL uses `SDL_VIDEODRIVER=dummy` when that variable is unset.

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
