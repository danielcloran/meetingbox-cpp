#ifndef AUDIO_HPP
#define AUDIO_HPP

#include "pv_porcupine.h"
#include <SDL2/SDL.h>

#include <stdio.h>
#include <thread>

namespace audio
{
    void initialize();
    void quit();

    namespace internal
    {
        static const char *ACCESS_KEY = "kjLI4wtcCkXEIaTxuYEi5AU6XbcGC+/3szekfDPquOqDnAJlI9BciQ==";
        const char *model_file_path = "../audio/porcupine_params.pv";
        const float sensitivity = 0.5f;
        pv_porcupine_t *porcupine;

#ifdef __APPLE__
        const char *keyword_file_path = "../audio/Meeting-Box_en_mac_v2_2_0.ppn";
#elif __linux__
        const char *keyword_file_path = "../audio/Meeting-Box_en_raspberry-pi_v2_2_0.ppn";
#endif

        void process_audio_data(void *userdata, Uint8 *stream, int len);

        void hotword_loop();
        extern std::atomic<bool> quit_;
        extern std::thread hotword_thread;
    }
}

#endif // AUDIO_HPP