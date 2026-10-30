#ifndef AUDIO_SIMPLE_HPP
#define AUDIO_SIMPLE_HPP

#include <pocketsphinx.h>
#include <stdio.h>
#include <thread>

namespace audio {
void initialize();
void quit();

namespace internal {
extern ps_decoder_t *ps_decoder;
extern ps_config_t *ps_config;

// Simple audio capture without SDL2
void start_audio_capture();
void stop_audio_capture();
void process_audio_data_simple();
} // namespace internal
} // namespace audio

#endif // AUDIO_SIMPLE_HPP
