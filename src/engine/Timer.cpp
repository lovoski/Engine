#include "Timer.hpp"

tTimer::tTimer() : deltaTime(0.0f), lastFrame(0.0f) {}
tTimer::~tTimer() {}

void tTimer::Initialize() {}

void tTimer::Tick() {
  deltaTime = glfwGetTime() - lastFrame;
  lastFrame = glfwGetTime();
}

float tTimer::DeltaTime() { return deltaTime; }