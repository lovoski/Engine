#include "Events.hpp"
#include "Engine.hpp"
#include "EditorWindows.hpp"

void WindowCloseCallback(GLFWwindow *window) { Core.Quit(); }

void MouseMoveCallback(GLFWwindow *window, double xpos, double ypos) {
  vec2 currentPos = vec2(static_cast<float>(xpos), static_cast<float>(ypos));

  EditorContext.io->AddMousePosEvent(currentPos.x, currentPos.y);
}

void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
  vec2 currentOffset = vec2(static_cast<float>(xoffset), static_cast<float>(yoffset));
  Event.MouseScrollOffset = currentOffset;

  EditorContext.io->AddMouseWheelEvent(currentOffset.x, currentOffset.y);
}

Events::Events() {}
Events::~Events() {}

void Events::Poll() {
  glfwPollEvents();

  double xpos, ypos;
  glfwGetCursorPos(&Core.Window(), &xpos, &ypos);
  MouseCurrentPosition.x = static_cast<float>(xpos);
  MouseCurrentPosition.y = static_cast<float>(ypos);
}

int Events::GetMouseButton(int button) {
  return glfwGetMouseButton(&Core.Window(), button);
}

int Events::GetKey(int key) {
  return glfwGetKey(&Core.Window(), key);
}

void Events::Initialize() {
  GLFWwindow &window = Core.Window();
  glfwSetWindowCloseCallback(&window, WindowCloseCallback);
  glfwSetCursorPosCallback(&window, MouseMoveCallback);
  glfwSetScrollCallback(&window, MouseScrollCallback);
}