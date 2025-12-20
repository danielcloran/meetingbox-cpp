#include "audio/audio.hpp"
#include <chrono>
#include <iostream>
#include <thread>

// Simple mock event queue to satisfy audio dependencies if needed
// or just run the audio initialization which starts the thread.

#include <limits.h>
#include <unistd.h>

int main(int argc, char **argv) {
  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    std::cout << "Current working dir: " << cwd << std::endl;
  } else {
    std::cerr << "getcwd() error" << std::endl;
  }
  std::cout << "Starting Media Demo (Hotword Detection)..." << std::endl;
  std::cout << "Say 'MeetingBox' to test detection." << std::endl;

  // Initialize audio system
  // This should start the audio capture thread and Porcupine
  audio::initialize();

  // Keep the main thread alive to allow audio processing
  std::cout << "Press Ctrl+C to exit." << std::endl;
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  audio::quit();
  return 0;
}
