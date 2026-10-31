#!/usr/bin/env python3
"""Start a timer and a paint process, push one frame, switch the LEDs with show.

The present loop stays inside timebox_mcp. This script only sends MCP tool calls.
"""

import base64
import json
import os
import subprocess
import sys
import time

WIDTH = 320
HEIGHT = 64
FRAME_BYTES = WIDTH * HEIGHT * 3
LID_PANEL = 2


def fail(message):
    raise SystemExit(message)


def solid_panel(panel, r, g, b):
    buf = bytearray(FRAME_BYTES)
    x0 = panel * 64
    for y in range(HEIGHT):
        for x in range(x0, x0 + 64):
            i = (y * WIDTH + x) * 3
            buf[i] = r
            buf[i + 1] = g
            buf[i + 2] = b
    return bytes(buf)


class Client:
    def __init__(self, bin_path):
        # Buffered binary pipes. A text-mode line buffer can hand back a short
        # read and desync the next JSON-RPC response.
        self.proc = subprocess.Popen(
            [bin_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
        )
        self.next_id = 1

    def _write(self, message):
        payload = json.dumps(message, separators=(",", ":")).encode() + b"\n"
        self.proc.stdin.write(payload)
        self.proc.stdin.flush()

    def request(self, method, params=None):
        want = self.next_id
        message = {"jsonrpc": "2.0", "id": want, "method": method}
        self.next_id += 1
        if params is not None:
            message["params"] = params
        self._write(message)
        # Skip a stray line so one extra stdout write cannot desync the call.
        for _ in range(6):
            line = self.proc.stdout.readline()
            if not line:
                fail("timebox_mcp closed stdout")
            try:
                parsed = json.loads(line.decode())
            except json.JSONDecodeError:
                fail("bad mcp line: %r" % line[:200])
            if parsed.get("id") == want:
                return parsed
        fail("no response for request %s" % want)

    def notify(self, method):
        self._write({"jsonrpc": "2.0", "method": method})

    def tool(self, name, arguments=None):
        response = self.request("tools/call", {"name": name, "arguments": arguments or {}})
        if "error" in response:
            fail(str(response["error"]))
        result = response["result"]
        text = result["content"][0]["text"]
        if result.get("isError"):
            fail(text)
        return json.loads(text)

    def close(self):
        self.proc.stdin.close()
        try:
            self.proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            self.proc.kill()
            self.proc.wait(timeout=5)


def main():
    bin_path = sys.argv[1] if len(sys.argv) > 1 else os.environ.get(
        "TIMEBOX_MCP", "bazel-bin/src/timebox_mcp"
    )
    if not os.path.isfile(bin_path):
        fail("missing %s\nbuild it with: bazel build //src:timebox_mcp" % bin_path)

    client = Client(bin_path)
    try:
        init = client.request(
            "initialize",
            {
                "protocolVersion": "2024-11-05",
                "capabilities": {},
                "clientInfo": {"name": "two-process-demo", "version": "0.1.0"},
            },
        )
        print("server", init["result"]["serverInfo"]["name"], init["result"]["serverInfo"]["version"])
        client.notify("notifications/initialized")

        meeting = client.tool(
            "timebox_start",
            {"kind": "timer", "name": "meeting", "seconds": 30, "focus": True},
        )
        splash = client.tool(
            "timebox_start",
            {"kind": "paint", "name": "splash", "focus": False},
        )
        print("started meeting id=%s splash id=%s focus=%s" % (
            meeting["id"], splash["id"], meeting["name"]))

        raw = solid_panel(LID_PANEL, 0, 220, 40)
        client.tool(
            "timebox_paint_frame",
            {"process_id": splash["id"], "frame_base64": base64.b64encode(raw).decode("ascii")},
        )

        names = {item["name"] for item in client.tool("timebox_list_processes")["processes"]}
        if names != {"meeting", "splash"}:
            fail("expected both processes, got %s" % sorted(names))

        splash_snap = client.tool("timebox_snapshot", {"name": "splash"})
        got = base64.b64decode(splash_snap["frame_base64"])
        lid = (0 * WIDTH + LID_PANEL * 64) * 3
        if got[lid:lid + 3] != bytes((0, 220, 40)):
            fail("paint frame did not land on the lid")
        if splash_snap["focused"]:
            fail("paint was focused; timer should still own the LEDs")

        shown = client.tool("timebox_show", {"name": "splash"})
        print("show", shown["focused_name"])
        status = client.tool("timebox_status")
        if status["focused_name"] != "splash" or status["process_count"] != 2:
            fail("show splash failed: %s" % status)

        client.tool("timebox_show", {"id": meeting["id"]})
        status = client.tool("timebox_status")
        if status["focused_name"] != "meeting" or status["process_count"] != 2:
            fail("show meeting stopped someone: %s" % status)
        meeting_snap = base64.b64decode(client.tool("timebox_snapshot")["frame_base64"])
        if meeting_snap == got:
            fail("timer frame matches the paint frame")
        wall = (63 * WIDTH + 10) * 3
        pixel = meeting_snap[wall:wall + 3]
        if pixel[0] != 0 or pixel[1] != 0 or pixel[2] < 200:
            fail("timer wall is not the blue countdown: %s" % list(pixel))

        deadline = time.time() + 5
        while True:
            status = client.tool("timebox_status")
            if status["present_count"] >= 1:
                break
            if time.time() > deadline:
                fail("present thread did not publish a frame")
            time.sleep(0.05)
        print("present_count", status["present_count"])

        client.tool("timebox_stop", {"name": "splash"})
        left = client.tool("timebox_list_processes")["processes"]
        if [item["name"] for item in left] != ["meeting"]:
            fail("splash stop removed the wrong process: %s" % left)
        client.tool("timebox_stop", {"id": meeting["id"]})
        if client.tool("timebox_list_processes")["processes"]:
            fail("processes left running")
        print("ok")
    finally:
        client.close()


if __name__ == "__main__":
    main()
