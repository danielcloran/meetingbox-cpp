#include "audio/audio.hpp"
#include "events/event_queue.hpp"
#include <SDL.h>
#include <chrono>
#include <fstream>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

namespace audio {
namespace internal {
const char *ACCESS_KEY =
    "kjLI4wtcCkXEIaTxuYEi5AU6XbcGC+/3szekfDPquOqDnAJlI9BciQ==";
const char *model_file_path =
    "media/src/audio/hotword_models/porcupine_params.pv";
const float sensitivity = 0.5f; // Lower sensitivity to reduce false positives
pv_porcupine_t *porcupine = nullptr;

#ifdef __APPLE__
const char *keyword_file_path =
    "media/src/audio/hotword_models/Meeting-Box_en_mac_v2_2_0.ppn";
#elif __linux__
const char *keyword_file_path =
    "media/src/audio/hotword_models/Meeting-Box_en_raspberry-pi_v2_2_0.ppn";
#endif

SDL_AudioDeviceID capture_device_id = 0;
std::vector<int16_t> audio_buffer;
std::mutex buffer_mutex;

void audio_callback(void *userdata, Uint8 *stream, int len) {
  // SDL gives us bytes, but we want 16-bit integers
  int16_t *samples = (int16_t *)stream;
  int sample_count = len / sizeof(int16_t);

  std::lock_guard<std::mutex> lock(buffer_mutex);
  audio_buffer.insert(audio_buffer.end(), samples, samples + sample_count);
}

std::thread processing_thread;
bool processing_running = false;

void process_audio_loop() {
  if (!porcupine)
    return;

  int32_t frame_length = pv_porcupine_frame_length();

  while (processing_running) {
    std::vector<int16_t> frame;

    {
      std::lock_guard<std::mutex> lock(buffer_mutex);
      if (audio_buffer.size() >= (size_t)frame_length) {
        // Extract one frame
        frame.insert(frame.end(), audio_buffer.begin(),
                     audio_buffer.begin() + frame_length);
        audio_buffer.erase(audio_buffer.begin(),
                           audio_buffer.begin() + frame_length);
      }
    }

    if (!frame.empty()) {
      int32_t keyword_index = -1;
      pv_status_t status =
          pv_porcupine_process(porcupine, frame.data(), &keyword_index);

      if (status == PV_STATUS_SUCCESS) {
        if (keyword_index >= 0) {
          std::cout << "Hotword Detected!" << std::endl;
          events::queue.enqueue(std::make_shared<HotwordDetectedEvent>());
        }
      } else {
        std::cerr << "Porcupine process error: " << pv_status_to_string(status)
                  << std::endl;
      }
    } else {
      // Wait a bit if not enough data
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }
}

void start_audio_capture() {
  std::cout << "Starting SDL2 audio capture..." << std::endl;

  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    std::cerr << "SDL Audio init failed: " << SDL_GetError() << std::endl;
    return;
  }

  SDL_AudioSpec want, have;
  SDL_zero(want);
  want.freq = 16000;          // Porcupine requires 16000 Hz
  want.format = AUDIO_S16SYS; // 16-bit signed integer
  want.channels = 1;          // Mono
  want.samples = 512;         // Buffer size
  want.callback = audio_callback;

  // Open default recording device
  capture_device_id = SDL_OpenAudioDevice(NULL, 1, &want, &have, 0);
  if (capture_device_id == 0) {
    std::cerr << "Failed to open audio device: " << SDL_GetError() << std::endl;
    return;
  }

  if (have.freq != 16000) {
    std::cerr << "Warning: Got different sample rate: " << have.freq
              << std::endl;
  }

  SDL_PauseAudioDevice(capture_device_id, 0); // Start recording

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
  std::cout << "Initializing audio system..." << std::endl;

  std::ifstream f(internal::model_file_path);
  if (!f.good()) {
    std::cerr << "Model file not found at: " << internal::model_file_path
              << std::endl;
  }
  std::ifstream f2(internal::keyword_file_path);
  if (!f2.good()) {
    std::cerr << "Keyword file not found at: " << internal::keyword_file_path
              << std::endl;
  }

  // Initialize Porcupine
  pv_status_t status =
      pv_porcupine_init(internal::ACCESS_KEY, internal::model_file_path,
                        1, // num_keywords
                        &internal::keyword_file_path, &internal::sensitivity,
                        &internal::porcupine);

  if (status != PV_STATUS_SUCCESS) {
    std::cerr << "Failed to initialize Porcupine: "
              << pv_status_to_string(status) << std::endl;
    return;
  }

  std::cout << "Porcupine initialized successfully" << std::endl;

  // Start audio capture
  internal::start_audio_capture();
}

void quit() {
  std::cout << "Quitting audio system..." << std::endl;

  // Stop audio capture
  internal::stop_audio_capture();

  // Cleanup Porcupine
  if (internal::porcupine) {
    pv_porcupine_delete(internal::porcupine);
    internal::porcupine = nullptr;
  }
}
} // namespace audio
