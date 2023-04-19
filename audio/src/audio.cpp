#include "audio/audio.hpp"
#include "events/event_queue.hpp"

#include <iostream>

const int SAMPLE_RATE = 16000; // 16 kHz
const int AUDIO_BUFFER_SIZE = 1024;

namespace audio
{
    namespace internal
    {
        std::atomic<bool> quit_(false);
        std::thread hotword_thread;

        void hotword_loop()
        {
            //     while (true)
            //     {
            //         const int16_t *pcm = get_next_audio_frame();
            //         int32_t keyword_index;
            //         const pv_status_t status = pv_porcupine_process(porcupine, pcm, &keyword_index);
            //         if (status != PV_STATUS_SUCCESS)
            //         {
            //             std::cerr << "Failed to process audio frame" << std::endl;
            //             // error handling logic
            //         }
            //         if (keyword_index != -1)
            //         {
            //             std::cout << "Hotword detected" << std::endl;
            //             events::queue.enqueue(std::make_shared<HotwordDetectedEvent>());
            //             // detection event logic/callback
            //         }
            //     }
        }

        void process_audio_data(void *userdata, Uint8 *stream, int len)
        {
            const int16_t *audio_data = reinterpret_cast<int16_t *>(stream);
            int length = len / 2;

            int32_t keyword_index = -1;
            pv_status_t status = pv_porcupine_process(internal::porcupine, audio_data, &keyword_index);
            if (status == PV_STATUS_SUCCESS && keyword_index != -1)
            {
                std::cout << "Hotword detected!" << std::endl;
            }
        }
    }

    void initialize()
    {
        // Initialize the hotword detector porcupine
        const pv_status_t status = pv_porcupine_init(
            internal::ACCESS_KEY,
            internal::model_file_path,
            1,
            &internal::keyword_file_path,
            &internal::sensitivity,
            &internal::porcupine);

        if (status != PV_STATUS_SUCCESS)
        {
            std::cerr << "Failed to initialize Porcupine" << std::endl;
        }

        // Set up the audio capture
        if (SDL_Init(SDL_INIT_AUDIO) < 0)
        {
            std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        }

        int num_audio_devices = SDL_GetNumAudioDevices(SDL_TRUE);
        if (num_audio_devices <= 0)
        {
            std::cerr << "No audio input devices found!" << std::endl;
        }

        const char *usb_microphone_name = nullptr;
        for (int i = 0; i < num_audio_devices; ++i)
        {
            const char *device_name = SDL_GetAudioDeviceName(i, SDL_TRUE);
            if (device_name != nullptr)
            {
                std::cout << "Audio device " << i << ": " << device_name << std::endl;
                // Replace 1 with the card number you got from arecord -l
                if (strstr(device_name, "hw:1,0") != nullptr)
                {
                    usb_microphone_name = device_name;
                    break;
                }
            }
        }

        if (usb_microphone_name == nullptr)
        {
            std::cerr << "USB microphone not found!" << std::endl;
        }

        // Configure audio specification
        SDL_AudioSpec audio_spec;
        audio_spec.freq = SAMPLE_RATE;
        audio_spec.format = AUDIO_S16LSB; // Signed 16-bit samples, Little Endian
        audio_spec.channels = 1;          // Mono
        audio_spec.samples = AUDIO_BUFFER_SIZE;
        audio_spec.userdata = nullptr;
        audio_spec.callback = internal::process_audio_data;

        // Open audio device for recording
        SDL_AudioDeviceID audio_device_id = SDL_OpenAudioDevice(usb_microphone_name, 1, &audio_spec, nullptr, 0);
        if (!audio_device_id)
        {
            std::cerr << "Failed to open audio device! SDL_Error: " << SDL_GetError() << std::endl;
        }

        // Start audio capture
        SDL_PauseAudioDevice(audio_device_id, 0);
    }

    void quit()
    {
        internal::quit_.store(true);
        if (internal::hotword_thread.joinable())
        {
            internal::hotword_thread.join();
        }
        pv_porcupine_delete(internal::porcupine);
    }
}