#include "Events.hpp"
#include "Engine.hpp"

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