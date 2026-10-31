"""Pro-only TimeBox timer demo: Gradio webpage + MCP tools.

Hugging Face returns HTTP 402 for Gradio Spaces on the free cpu-basic tier.
The Action deploys the Static Space in ../timebox-timer-demo/ instead.
Run this app on HF Pro or another Gradio host.

Mirrors meetingbox-cpp timer faces (row-fade walls, spiral lid).
Strip order is Left | Front | Top | Right | Back. The lid is panel index 2.
"""
from __future__ import annotations

import threading
import time
from typing import Any

import gradio as gr
import numpy as np
from PIL import Image

# Five 64x64 panels. Lid is the middle panel (graphics::kTimeboxTopPanel).
PANEL = 64
TOP = 2
SPIRAL_N = PANEL * PANEL
# graphics::kBoxBackground is {0x10, 0x10, 0x10}.
BOX_BG = 0x10
# Unfolded cross, lid in the center. dest is (strip_index, x, y).
#        Back
#   Left Top  Right
#        Front
_PLACEMENTS = (
    (2, 64, 64),
    (0, 0, 64),
    (3, 128, 64),
    (1, 64, 128),
    (4, 64, 0),
)


def _spiral_ranks() -> np.ndarray:
    """Clockwise spiral rank. 0 is the outer start at (x=0, y=0)."""
    ranks = np.zeros((PANEL, PANEL), dtype=np.int32)
    rank = 0
    top, bottom, left, right = 0, PANEL - 1, 0, PANEL - 1
    while top <= bottom and left <= right:
        for x in range(left, right + 1):
            ranks[top, x] = rank
            rank += 1
        top += 1
        for y in range(top, bottom + 1):
            ranks[y, right] = rank
            rank += 1
        right -= 1
        if top <= bottom:
            for x in range(right, left - 1, -1):
                ranks[bottom, x] = rank
                rank += 1
            bottom -= 1
        if left <= right:
            for y in range(bottom, top - 1, -1):
                ranks[y, left] = rank
                rank += 1
            left += 1
    return ranks


RANKS = _spiral_ranks()
_LOCK = threading.Lock()
_PROCESSES: dict[str, dict[str, Any]] = {}
_FOCUSED: str | None = None


def _row_blue(row: int, remaining: float) -> int:
    """Blue channel for one wall row. Mirrors graphics::timebox_row_blue (float32)."""
    if row < 0 or row >= PANEL:
        return 0
    rem = np.float32(remaining)
    if rem < np.float32(0):
        rem = np.float32(0)
    elif rem > np.float32(1):
        rem = np.float32(1)
    row_pct = np.float32(PANEL - row) / np.float32(PANEL)
    if row_pct >= np.float32(1):
        brightness = np.float32(1) if rem >= np.float32(1) else np.float32(0)
    else:
        brightness = np.float32(1) - (np.float32(1) - rem) / (np.float32(1) - row_pct)
    if brightness < np.float32(0):
        brightness = np.float32(0)
    elif brightness > np.float32(1):
        brightness = np.float32(1)
    return int(np.float32(255) * brightness)


def _dark_count(remaining: float) -> int:
    """How many outer spiral ranks are already dark. Mirrors the C++ cast."""
    rem = np.float32(remaining)
    if rem < np.float32(0):
        rem = np.float32(0)
    elif rem > np.float32(1):
        rem = np.float32(1)
    if rem <= np.float32(0):
        return SPIRAL_N
    if rem >= np.float32(1):
        return 0
    return int((np.float32(1) - rem) * np.float32(SPIRAL_N))


def _lid(remaining: float) -> np.ndarray:
    """64x64 lid. Outer ranks go dark first; the center stays lit longest."""
    dark = _dark_count(remaining)
    span = SPIRAL_N - dark
    out = np.zeros((PANEL, PANEL), dtype=np.uint8)
    if span > 0:
        denom = span - 1 if span > 1 else 1
        mask = RANKS >= dark
        out[mask] = 40 + (215 * (RANKS[mask] - dark)) // denom
    return out


def paint_strip(remaining: float) -> np.ndarray:
    """320x64 RGB strip. Walls are the row-fade; panel 2 is overwritten with the spiral."""
    img = np.zeros((PANEL, 5 * PANEL, 3), dtype=np.uint8)
    col = np.array([_row_blue(y, remaining) for y in range(PANEL)], dtype=np.uint8)
    img[:, :, 2] = col[:, None]
    x0 = TOP * PANEL
    img[:, x0 : x0 + PANEL, 2] = _lid(remaining)
    return img


def paint_cross(strip: np.ndarray) -> np.ndarray:
    """192x192 unfolded box. Four corners stay the firmware background."""
    box = np.full((3 * PANEL, 3 * PANEL, 3), BOX_BG, dtype=np.uint8)
    for strip_index, dest_x, dest_y in _PLACEMENTS:
        panel = strip[:, strip_index * PANEL : (strip_index + 1) * PANEL]
        box[dest_y : dest_y + PANEL, dest_x : dest_x + PANEL] = panel
    return box


def _up(arr: np.ndarray, scale: int) -> Image.Image:
    return Image.fromarray(arr).resize(
        (arr.shape[1] * scale, arr.shape[0] * scale), Image.NEAREST
    )


def _remaining_frac(proc: dict[str, Any]) -> float:
    total = float(proc["total_s"])
    if total <= 0:
        return 0.0
    if proc.get("start") is None:
        return 1.0
    rem_s = max(0.0, total - (time.time() - float(proc["start"])))
    return rem_s / total


def _firmware_paint_checks() -> None:
    """Fail fast if the hosted paint drifts from media/src/graphics/timer_paint_test.cpp."""
    assert _row_blue(0, 1.0) == 255
    assert _row_blue(63, 1.0) == 255
    assert _row_blue(0, 0.5) == 0
    assert _row_blue(32, 0.5) == 0
    assert _row_blue(63, 0.5) == 125
    assert _row_blue(60, 0.5) == 118
    assert _row_blue(48, 0.75) == 169
    assert _row_blue(63, 0.0) == 0
    assert _row_blue(10, 0.5) < _row_blue(60, 0.5)
    assert int(RANKS[0, 0]) == 0
    assert int(RANKS[0, 63]) == 63
    assert int(RANKS.max()) == SPIRAL_N - 1
    assert int(np.unique(RANKS).size) == SPIRAL_N
    cy, cx = np.unravel_index(int(RANKS.argmax()), RANKS.shape)
    full_lid = _lid(1.0)
    half_lid = _lid(0.5)
    assert int(full_lid[0, 0]) == 40
    assert int(full_lid[cy, cx]) == 255
    assert int(full_lid[0, 0]) < int(full_lid[cy, cx])
    assert int(half_lid[0, 0]) == 0
    assert int(half_lid[cy, cx]) > 200
    assert int(_lid(0.0)[cy, cx]) == 0
    half = paint_strip(0.5)
    assert half.shape == (PANEL, 5 * PANEL, 3)
    assert int(half[0, 10, 2]) == 0
    assert int(half[63, 10, 2]) == 125
    assert int(half[0, TOP * PANEL, 2]) == 0
    for panel in (0, 1, 3, 4):
        assert int(half[63, panel * PANEL + 8, 2]) == 125
    cross = paint_cross(half)
    assert cross.shape == (3 * PANEL, 3 * PANEL, 3)
    assert int(cross[0, 0, 0]) == BOX_BG
    assert int(cross[160, 160, 0]) == BOX_BG
    assert np.array_equal(cross[0:PANEL, PANEL : 2 * PANEL], half[:, 4 * PANEL : 5 * PANEL])
    assert np.array_equal(
        cross[2 * PANEL : 3 * PANEL, PANEL : 2 * PANEL], half[:, PANEL : 2 * PANEL]
    )
    assert np.array_equal(
        cross[PANEL : 2 * PANEL, PANEL : 2 * PANEL], half[:, 2 * PANEL : 3 * PANEL]
    )


_firmware_paint_checks()


def timebox_start(name: str, seconds: float = 30.0) -> str:
    """Start (or restart) a named timer process.

    Does not steal the display when another process is already focused.
    Call timebox_show to point the faces at this timer.

    Args:
        name: Process name (e.g. demo-30s).
        seconds: Meeting length in seconds.
    """
    global _FOCUSED
    name = (name or "timer").strip() or "timer"
    seconds = float(max(1.0, min(float(seconds), 86400.0)))
    with _LOCK:
        _PROCESSES[name] = {"total_s": seconds, "start": time.time(), "kind": "timer"}
        if _FOCUSED is None:
            _FOCUSED = name
        focused = _FOCUSED
    return f"started {name} for {seconds:.0f}s (focused={focused})"


def timebox_stop(name: str) -> str:
    """Stop and remove a named process.

    Args:
        name: Process name to stop.
    """
    global _FOCUSED
    name = (name or "").strip()
    with _LOCK:
        if name not in _PROCESSES:
            return f"no process named {name!r}"
        del _PROCESSES[name]
        if _FOCUSED == name:
            _FOCUSED = next(iter(_PROCESSES), None)
        focused = _FOCUSED
    return f"stopped {name} (focused={focused})"


def timebox_show(name: str) -> str:
    """Focus the display on a named running process.

    Args:
        name: Process name to show.
    """
    global _FOCUSED
    name = (name or "").strip()
    with _LOCK:
        if name not in _PROCESSES:
            return f"no process named {name!r}"
        _FOCUSED = name
    return f"showing {name}"


def timebox_list_processes() -> str:
    """List running demo processes and which one is focused."""
    with _LOCK:
        if not _PROCESSES:
            return "no processes"
        lines = []
        for n, p in _PROCESSES.items():
            rem = _remaining_frac(p) * float(p["total_s"])
            mark = "*" if n == _FOCUSED else " "
            lines.append(f"{mark} {n}: {p['kind']} {rem:.1f}s / {p['total_s']:.0f}s")
        return "\n".join(lines)


def timebox_status() -> str:
    """Return focus and remaining time for the focused timer."""
    with _LOCK:
        if _FOCUSED is None or _FOCUSED not in _PROCESSES:
            return "idle (nothing focused)"
        p = _PROCESSES[_FOCUSED]
        rem = _remaining_frac(p) * float(p["total_s"])
        return f"focused={_FOCUSED} remaining={rem:.1f}s total={p['total_s']:.0f}s"


def timebox_snapshot() -> Image.Image:
    """Return the current 320x64 LED strip, upscaled 4x, for the focused process."""
    with _LOCK:
        if _FOCUSED is None or _FOCUSED not in _PROCESSES:
            rem = 0.0
        else:
            rem = _remaining_frac(_PROCESSES[_FOCUSED])
    return _up(paint_strip(rem), 4)


def _focused_remaining() -> float:
    with _LOCK:
        if _FOCUSED is None or _FOCUSED not in _PROCESSES:
            return 0.0
        return _remaining_frac(_PROCESSES[_FOCUSED])


def _ui_state_tick() -> tuple[Image.Image, Image.Image, str, str]:
    status = timebox_status()
    listing = timebox_list_processes()
    rem = _focused_remaining()
    strip = paint_strip(rem)
    return _up(strip, 4), _up(paint_cross(strip), 2), status, listing


def _ui_start(name: str, seconds: float) -> tuple[Image.Image, Image.Image, str, str]:
    msg = timebox_start(name, seconds)
    strip_img, cross_img, status, listing = _ui_state_tick()
    return strip_img, cross_img, status + "\n" + msg, listing


def _ui_stop(name: str) -> tuple[Image.Image, Image.Image, str, str]:
    msg = timebox_stop(name)
    strip_img, cross_img, status, listing = _ui_state_tick()
    return strip_img, cross_img, status + "\n" + msg, listing


def _ui_show(name: str) -> tuple[Image.Image, Image.Image, str, str]:
    msg = timebox_show(name)
    strip_img, cross_img, status, listing = _ui_state_tick()
    return strip_img, cross_img, status + "\n" + msg, listing


CSS = """
#col-container { max-width: 1400px; margin: 0 auto; }
.dark .gradio-container { color: var(--body-text-color); }
"""

with gr.Blocks(title="TimeBox timer demo", theme=gr.themes.Soft(), css=CSS) as demo:
    with gr.Column(elem_id="col-container"):
        gr.Markdown(
            "# TimeBox timer demo (Gradio + MCP, HF Pro)\n"
            "Software preview of the TimeBox MCP timer faces from "
            "[meetingbox-cpp](https://github.com/danielcloran/meetingbox-cpp). "
            "Not the physical LEDs — same paint math (row-fade walls, spiral lid "
            "on panel 2 / Top). The free public demo is the Static Space; "
            "this app needs HF Pro or another host.\n\n"
            "Strip: Left | Front | Top | Right | Back. Unfolded box, lid in the center: "
            "Back above, Front below, Left and Right on the sides.\n\n"
            "MCP: `/gradio_api/mcp/` — tools `timebox_start`, `timebox_stop`, "
            "`timebox_show`, `timebox_list_processes`, `timebox_status`, `timebox_snapshot`."
        )
        with gr.Row():
            name = gr.Textbox(value="demo-30s", label="Process name")
            seconds = gr.Slider(minimum=5, maximum=3600, value=30, step=5, label="Seconds")
        with gr.Row():
            b_start = gr.Button("Start", variant="primary")
            b_show = gr.Button("Show")
            b_stop = gr.Button("Stop")
        status = gr.Textbox(label="Status", lines=2)
        listing = gr.Textbox(label="Processes", lines=4)
        strip = gr.Image(label="Strip 320×64 @4× (Left | Front | Top | Right | Back)", type="pil")
        cross = gr.Image(label="Unfolded box @2× (Back / Left·Top·Right / Front)", type="pil")

        outs = [strip, cross, status, listing]
        # Webpage-only events. api_name=False keeps them off the MCP tool list.
        b_start.click(_ui_start, [name, seconds], outs, api_name=False)
        b_stop.click(_ui_stop, [name], outs, api_name=False)
        b_show.click(_ui_show, [name], outs, api_name=False)
        gr.Timer(0.1).tick(
            _ui_state_tick, None, outs, api_name=False, show_progress="hidden", queue=False
        )
        demo.load(_ui_state_tick, None, outs, api_name=False, show_progress="hidden")

        with gr.Accordion("MCP / API tools", open=False):
            api_name = gr.Textbox(value="demo-30s", label="name")
            api_seconds = gr.Number(value=30, label="seconds")
            api_out = gr.Textbox(label="result")
            api_img = gr.Image(type="pil", label="snapshot")
            gr.Button("timebox_start").click(
                timebox_start, [api_name, api_seconds], api_out, api_name="timebox_start"
            )
            gr.Button("timebox_stop").click(
                timebox_stop, [api_name], api_out, api_name="timebox_stop"
            )
            gr.Button("timebox_show").click(
                timebox_show, [api_name], api_out, api_name="timebox_show"
            )
            gr.Button("timebox_list_processes").click(
                timebox_list_processes, None, api_out, api_name="timebox_list_processes"
            )
            gr.Button("timebox_status").click(
                timebox_status, None, api_out, api_name="timebox_status"
            )
            gr.Button("timebox_snapshot").click(
                timebox_snapshot, None, api_img, api_name="timebox_snapshot"
            )

if __name__ == "__main__":
    demo.launch(mcp_server=True)
