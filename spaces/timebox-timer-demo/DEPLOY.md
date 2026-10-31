# Deploying the hosted TimeBox demo

Hugging Face serves **Static** Spaces on the free plan. Gradio and Docker Spaces on cpu-basic return HTTP 402 unless the account has HF Pro.

The Space `danielcloran/timebox-timer-demo` already exists as Static. The workflow uploads this directory. It must not recreate the Space as Gradio.

## Free path (this directory)

1. Add a Hugging Face **write** token as the GitHub Actions secret `HF_TOKEN` on `meetingbox-cpp` (Settings → Secrets and variables → Actions). Do not commit the token.
2. Optional repository variable: `HF_SPACE_ID` (default `danielcloran/timebox-timer-demo`).
3. Merge to `master` or `main` with changes under `spaces/timebox-timer-demo/`, or run **Deploy TimeBox demo Space** (`workflow_dispatch`). The workflow runs `hf repos create … --type space --space-sdk static --public --exist-ok` and then `hf upload` of this directory.
4. Space: https://huggingface.co/spaces/danielcloran/timebox-timer-demo
5. App: https://danielcloran-timebox-timer-demo.static.hf.space/

## Gradio + MCP (HF Pro, or another host)

Not deployed by the Action. The app is [`spaces/timebox-timer-demo-gradio/`](../timebox-timer-demo-gradio/). On an HF Pro account, or any other host that can run Gradio, create a Gradio Space and upload that directory. `app.py` calls `demo.launch(mcp_server=True)`.

MCP path on that host: `/gradio_api/mcp/`

Tools: `timebox_start`, `timebox_stop`, `timebox_show`, `timebox_list_processes`, `timebox_status`, `timebox_snapshot`.
