#pragma once

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
  float DeltaTime();

  float GetTime();

private:
  tTimer();
  float lastFrame, deltaTime;
};

static tTimer &Timer = tTimer::Ref();
