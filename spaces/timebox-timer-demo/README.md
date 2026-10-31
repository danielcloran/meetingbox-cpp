---
title: TimeBox Timer Demo
emoji: ⏱️
colorFrom: blue
colorTo: indigo
sdk: gradio
sdk_version: 5.49.1
app_file: app.py
pinned: false
short_description: Hosted TimeBox timer UI + MCP tools
python_version: "3.12"
tags:
  - mcp-server-track
---

Hosted software preview of the TimeBox MCP timer faces from
[meetingbox-cpp](https://github.com/danielcloran/meetingbox-cpp).

The strip is Left | Front | Top | Right | Back. Side walls use the row-fade
countdown. The lid is panel index 2 (Top) and drains along a clockwise spiral.
The unfolded view puts that lid in the center:

```text
       Back
 Left  Top  Right
       Front
```

## Webpage

Open this Space and press **Start** for a 30s timer.

https://huggingface.co/spaces/danielcloran/timebox-timer-demo

## MCP

```text
https://danielcloran-timebox-timer-demo.hf.space/gradio_api/mcp/
```

API routes: `timebox_start`, `timebox_stop`, `timebox_show`, `timebox_list_processes`, `timebox_status`, `timebox_snapshot`.

On a Space, Gradio prefixes those MCP tool names with the Space id, so a client sees `timebox_timer_demo_timebox_start` (and the same prefix on the other five).
