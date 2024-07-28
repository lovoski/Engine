#include "Events.hpp"
#include "Engine.hpp"
#include "EditorWindows.hpp"

void WindowCloseCallback(GLFWwindow *window) { Core.Quit(); }

void MouseMoveCallback(GLFWwindow *window, double xpos, double ypos) {
  vec2 currentPos = vec2(static_cast<float>(xpos), static_cast<float>(ypos));

  EditorContext.io->AddMousePosEvent(currentPos.x, currentPos.y);
}

void MouseScrollCallback(GLFWwindow *window, double xoffset, double yoffset) {
  Event._currentScrollOffset = vec2(static_cast<float>(xoffset), static_cast<float>(yoffset));
  // Console.Log("push to actions, scorll offset y=%f\n", yoffset);
  Event.actions.push_back({Action::ActionType::MOUSE_SCROLL, &Event._currentScrollOffset});
  EditorContext.io->AddMouseWheelEvent(Event._currentScrollOffset.x, Event._currentScrollOffset.y);
}

Events::Events() {}
Events::~Events() {}

void Events::Poll() {
  actions.clear();
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