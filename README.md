# meetingbox-cpp

C++ firmware for the MeetingBox LED cube.

## Hosted demo

A public 30s timer preview (Gradio webpage + MCP) lives in [`spaces/timebox-timer-demo/`](spaces/timebox-timer-demo/). It uses the same row-fade walls and spiral lid as the device, with the lid on panel index 2 (Top) of the Left | Front | Top | Right | Back strip.

- Webpage: https://huggingface.co/spaces/danielcloran/timebox-timer-demo
- MCP: https://danielcloran-timebox-timer-demo.hf.space/gradio_api/mcp/

Pushes to `master` or `main` that touch `spaces/timebox-timer-demo/**`, and manual runs of [`.github/workflows/deploy-hf-space.yml`](.github/workflows/deploy-hf-space.yml), deploy that directory. The Action requires a repository secret named `HF_TOKEN` (a Hugging Face write token). See [`spaces/timebox-timer-demo/DEPLOY.md`](spaces/timebox-timer-demo/DEPLOY.md).
