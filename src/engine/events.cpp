#include "events.hpp"
#include "engine.hpp"

void WindowCloseCallback(GLFWwindow *window) {
  Core.Quit();
}

Events::Events() {}
Events::~Events() {}

void Events::Poll() {
  glfwPollEvents();
}

void Events::Initialize() {
  GLFWwindow &window = Core.Window();
  glfwSetWindowCloseCallback(&window, WindowCloseCallback);
}