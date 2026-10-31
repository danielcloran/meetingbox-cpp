# meetingbox-cpp

C++ firmware for the MeetingBox LED cube.

## Hosted demo

The free public preview is a Static Space. It paints the same row-fade walls and spiral lid as the device, with the lid on panel index 2 (Top) of the Left | Front | Top | Right | Back strip.

- Space: https://huggingface.co/spaces/danielcloran/timebox-timer-demo
- App: https://danielcloran-timebox-timer-demo.static.hf.space/

Source is [`spaces/timebox-timer-demo/`](spaces/timebox-timer-demo/). Pushes to `master` or `main` that touch that directory, and manual runs of [`.github/workflows/deploy-hf-space.yml`](.github/workflows/deploy-hf-space.yml), upload it. The Action needs a repository secret named `HF_TOKEN` (a Hugging Face write token). See [`spaces/timebox-timer-demo/DEPLOY.md`](spaces/timebox-timer-demo/DEPLOY.md).

Gradio plus MCP is in [`spaces/timebox-timer-demo-gradio/`](spaces/timebox-timer-demo-gradio/). Hugging Face returns HTTP 402 for Gradio and Docker Spaces on free cpu-basic, so that app needs HF Pro or another host. The Action does not deploy it.
