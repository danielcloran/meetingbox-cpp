# TimeBox timer demo (Gradio + MCP)

Pro-only. Hugging Face returns HTTP 402 for new Gradio and Docker Spaces on the free cpu-basic tier. The GitHub Action does **not** upload this directory.

The free public demo is the Static Space in [`../timebox-timer-demo/`](../timebox-timer-demo/):

- https://huggingface.co/spaces/danielcloran/timebox-timer-demo
- https://danielcloran-timebox-timer-demo.static.hf.space/

## Run this app

On HF Pro, or any other Gradio host, upload this directory to a Space with `sdk: gradio` (or run `python app.py` locally). `demo.launch(mcp_server=True)` exposes:

`timebox_start`, `timebox_stop`, `timebox_show`, `timebox_list_processes`, `timebox_status`, `timebox_snapshot`

MCP path: `/gradio_api/mcp/`

`requirements.txt` lists numpy and pillow only. Do not pin `gradio` or `huggingface_hub`; the Space runtime provides them. The Space image also installs the Gradio MCP extra, so do not add `mcp` here.
