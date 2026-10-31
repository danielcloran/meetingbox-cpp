# Deploying the hosted TimeBox demo

1. Add a Hugging Face **write** token as the GitHub Actions secret `HF_TOKEN` on `meetingbox-cpp` (Settings → Secrets and variables → Actions). Do not commit the token.
2. Optional repository variable: `HF_SPACE_ID` (default `danielcloran/timebox-timer-demo`).
3. Merge to `master` or `main` with changes under `spaces/timebox-timer-demo/`, or run **Deploy TimeBox demo Space** (`workflow_dispatch`). The workflow creates the public Gradio Space if it is missing and uploads this directory.
4. Webpage: https://huggingface.co/spaces/danielcloran/timebox-timer-demo
5. MCP: `https://danielcloran-timebox-timer-demo.hf.space/gradio_api/mcp/`
