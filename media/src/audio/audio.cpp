#include "audio/audio.hpp"
#include "events/event_queue.hpp"
#include <SDL2/SDL.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <pocketsphinx.h>
#include <thread>
#include <vector>

namespace audio {
namespace internal {

ps_decoder_t *ps_decoder = nullptr;
ps_config_t *ps_config = nullptr;

SDL_AudioDeviceID capture_device_id = 0;
std::vector<int16_t> audio_buffer;
std::mutex buffer_mutex;

void audio_callback(void *userdata, Uint8 *stream, int len) {
  int16_t *samples = (int16_t *)stream;
  int sample_count = len / sizeof(int16_t);

  std::lock_guard<std::mutex> lock(buffer_mutex);
  audio_buffer.insert(audio_buffer.end(), samples, samples + sample_count);
}

std::thread processing_thread;
bool processing_running = false;

void process_audio_loop() {
  if (!ps_decoder)
    return;

  // Start initial utterance
  if (ps_start_utt(ps_decoder) < 0) {
    std::cerr << "Failed to start utterance" << std::endl;
    return;
  }

  while (processing_running) {
    std::vector<int16_t> chunk;
    {
      std::lock_guard<std::mutex> lock(buffer_mutex);
      if (!audio_buffer.empty()) {
        chunk.swap(audio_buffer);
      }
    }

    if (!chunk.empty()) {
      ps_process_raw(ps_decoder, chunk.data(), chunk.size(), FALSE, FALSE);

      int32 score;
      const char *hyp = ps_get_hyp(ps_decoder, &score);

      if (hyp != nullptr) {
        std::string detection(hyp);
        // Check if detection matches "box" (case insensitive usually,
        // but keyphrase is exact)
        if (detection.find("box") != std::string::npos) {
          std::cout << "Hotword 'Box' Detected!" << std::endl;
          events::queue.enqueue(std::make_shared<HotwordDetectedEvent>());

          // Restart utterance after detection to reset hypothesis
          ps_end_utt(ps_decoder);
          if (ps_start_utt(ps_decoder) < 0) {
            std::cerr << "Failed to restart utterance" << std::endl;
          }
        }
      }
    } else {
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
  }

  ps_end_utt(ps_decoder);
}

void start_audio_capture() {
  std::cout << "Starting SDL2 audio capture..." << std::endl;

  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    std::cerr << "SDL Audio init failed: " << SDL_GetError() << std::endl;
    return;
  }

  SDL_AudioSpec want, have;
  SDL_zero(want);
  want.freq = 16000;
  want.format = AUDIO_S16SYS;
  want.channels = 1;
  want.samples = 1024;
  want.callback = audio_callback;

  capture_device_id = SDL_OpenAudioDevice(NULL, 1, &want, &have, 0);
  if (capture_device_id == 0) {
    std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
    return;
  }

  if (have.freq != 16000) {
    std::cerr << "Warning: Got different sample rate: " << have.freq
              << std::endl;
  }

  SDL_PauseAudioDevice(capture_device_id, 0);

  processing_running = true;
  processing_thread = std::thread(process_audio_loop);
}

void stop_audio_capture() {
  std::cout << "Stopping audio capture..." << std::endl;

  processing_running = false;
  if (processing_thread.joinable()) {
    processing_thread.join();
  }

  if (capture_device_id != 0) {
    SDL_CloseAudioDevice(capture_device_id);
    capture_device_id = 0;
  }
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

} // namespace internal

void initialize() {
  std::cout << "Initializing PocketSphinx..." << std::endl;

  internal::ps_config = ps_config_init(NULL);
  ps_default_search_args(internal::ps_config);

  // Set explicit model paths for Bazel runfiles
  ps_config_set_str(internal::ps_config, "hmm",
                    "../+_repo_rules+pocketsphinx/model/en-us/en-us");
  ps_config_set_str(
      internal::ps_config, "dict",
      "../+_repo_rules+pocketsphinx/model/en-us/cmudict-en-us.dict");

  // Set the keyphrase to detect
  ps_config_set_str(internal::ps_config, "keyphrase", "box");
  ps_config_set_float(internal::ps_config, "kws_threshold", 1e-20);

  internal::ps_decoder = ps_init(internal::ps_config);
  if (!internal::ps_decoder) {
    std::cerr << "PocketSphinx init failed!" << std::endl;
    ps_config_free(internal::ps_config);
    return;
  }

  std::cout << "PocketSphinx initialized successfully" << std::endl;
  internal::start_audio_capture();
}

void quit() {
  std::cout << "Quitting audio system..." << std::endl;
  internal::stop_audio_capture();

  if (internal::ps_decoder) {
    ps_free(internal::ps_decoder);
    internal::ps_decoder = nullptr;
  }
  if (internal::ps_config) {
    ps_config_free(internal::ps_config);
    internal::ps_config = nullptr;
  }
}
} // namespace audio
