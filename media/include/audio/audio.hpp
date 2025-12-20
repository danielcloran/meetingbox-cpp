#ifndef AUDIO_SIMPLE_HPP
#define AUDIO_SIMPLE_HPP

#include "pv_porcupine.h"
#include <stdio.h>
#include <thread>

namespace audio
{
    void initialize();
    void quit();

    namespace internal
    {
        extern const char *ACCESS_KEY;
        extern const char *model_file_path;
        extern const float sensitivity;
        extern pv_porcupine_t *porcupine;
        extern const char *keyword_file_path;
        
        // Simple audio capture without SDL2
        void start_audio_capture();
        void stop_audio_capture();
        void process_audio_data_simple();
    }
}

#endif // AUDIO_SIMPLE_HPP
