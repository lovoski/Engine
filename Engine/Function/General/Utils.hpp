#pragma once

#include <chrono>
#include <iostream>
#include <fstream>
#include <random>

namespace aEngine {

// Returns a double between 0.0 and 1.0
double RandDouble() {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dis(0.0, 1.0);
  return dis(gen);
}

class Timer {
public:
  Timer() { Reset(); }

  ~Timer() {}

  // Resets the timer to the current time
  void Reset() { startTime = std::chrono::high_resolution_clock::now(); }

  // Returns the elapsed time in milliseconds since the timer started or was
  // last reset
  double ElapsedMilliseconds() const {
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = endTime - startTime;
    return duration.count();
  }

  // Returns the elapsed time in seconds since the timer started or was last
  // reset
  double ElapsedSeconds() const {
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = endTime - startTime;
    return duration.count();
  }

private:
  std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
};

}; // namespace aEngine