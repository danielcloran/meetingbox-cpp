---
title: TimeBox Timer Demo
emoji: ⏱️
colorFrom: blue
colorTo: indigo
sdk: static
pinned: false
short_description: Live TimeBox timer face preview (static)
---

Free static preview of the TimeBox timer faces from
[meetingbox-cpp](https://github.com/danielcloran/meetingbox-cpp).

The strip is Left | Front | Top | Right | Back. Side walls use the row-fade
countdown. The lid is panel index 2 (Top) and drains along a clockwise spiral.
The unfolded view puts that lid in the center, with Back above and Front below.

Open the Space and press **Start** for a 30s timer.

- App: https://danielcloran-timebox-timer-demo.static.hf.space/
- Space: https://huggingface.co/spaces/danielcloran/timebox-timer-demo

Gradio and Docker Spaces on Hugging Face’s free cpu-basic hardware return
HTTP 402 (Pro required). This Space stays on the Static SDK so it remains
free. The Gradio + MCP app is not served here.
