#ifndef TIMER_HPP
#define TIMER_HPP

#include "global.hpp"

class tTimer {
public:
  tTimer(const tTimer &) = delete;
  ~tTimer();
  tTimer &operator=(tTimer &) = delete;

  static tTimer &Ref() {
    static tTimer reference;
    return reference;
  }

  void Tick();
  void Initialize();
  float DeltaTime() {}

private:
  tTimer();
  float lastFrame, deltaTime;
};

static tTimer &Timer = tTimer::Ref();

#endif