#include "audio/audio.hpp"
#include <chrono>
#include <iostream>
#include <thread>

// Simple mock event queue to satisfy audio dependencies if needed
// or just run the audio initialization which starts the thread.

#include <limits.h>
#include <unistd.h>

#include <atomic>
#include <csignal>

std::atomic<bool> running(true);

void signal_handler(int signal) {
  if (signal == SIGINT) {
    std::cout << "\nReceived SIGINT. Exiting..." << std::endl;
    running = false;
  }
}

int main(int argc, char **argv) {
  std::signal(SIGINT, signal_handler);

  char cwd[PATH_MAX];
  if (getcwd(cwd, sizeof(cwd)) != NULL) {
    std::cout << "Current working dir: " << cwd << std::endl;
  } else {
    std::cerr << "getcwd() error" << std::endl;
  }
  std::cout << "Starting Media Demo (Hotword Detection)..." << std::endl;
  std::cout << "Say 'Box' to test detection." << std::endl;

  // Initialize audio system
  // This should start the audio capture thread and PocketSphinx
  audio::initialize();

  // Keep the main thread alive to allow audio processing
  std::cout << "Press Ctrl+C to exit." << std::endl;
  while (running) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  audio::quit();
  return 0;
}
