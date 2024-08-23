#include "Engine.hpp"

namespace aEngine {

Engine::Engine(int width, int height)
    : windowWidth(width), windowHeight(height) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  auto &monitor = *glfwGetVideoMode(glfwGetPrimaryMonitor());
  glfwWindowHint(GLFW_RED_BITS, monitor.redBits);
  glfwWindowHint(GLFW_BLUE_BITS, monitor.blueBits);
  glfwWindowHint(GLFW_GREEN_BITS, monitor.greenBits);

  window = glfwCreateWindow(width, height, "Engine", NULL, NULL);
  if (!window) {
    std::cout << "Failed to create GLFW window" << std::endl;
    return;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to load GLAD" << std::endl;
    return;
  }

  // pass the pointer of current windows to context
  GWORLD.Context.sceneWindowSize = glm::vec2(width, height);
  GWORLD.Context.sceneWindowPos = glm::vec2(0.0f);
  GWORLD.Context.window = window;
  GWORLD.Context.engine = this;

  // setup the user pointer so we can register events
  glfwSetWindowUserPointer(window, this);

  MouseMoveCallbacks.push_back([](Engine *engine, double x, double y) {
    // update mouse current position
    GWORLD.Context.currentMousePosition = glm::vec2(x, y);
  });
  MouseScrollCallbacks.push_back([](Engine *engine, double x, double y) {
    engine->_mouseScrollOffsets = glm::vec2(x, y);
    engine->ActionQueue.push_back({ACTION_TYPE::MOUSE_SCROLL, (void*)&engine->_mouseScrollOffsets});
  });

  // setup the callbacks
  glfwSetCursorPosCallback(window, [](GLFWwindow *wnd, double x, double y) {
    auto engine = static_cast<Engine *>(glfwGetWindowUserPointer(wnd));
    for (auto mmcb : engine->MouseMoveCallbacks)
      mmcb(engine, x, y);
  });
  glfwSetWindowCloseCallback(window, [](GLFWwindow *wnd) {
    auto engine = static_cast<Engine *>(glfwGetWindowUserPointer(wnd));
    for (auto wccb : engine->WindowCloseCallbacks)
      wccb(engine);
  });
  glfwSetScrollCallback(window, [](GLFWwindow *wnd, double x, double y) {
    auto engine = static_cast<Engine *>(glfwGetWindowUserPointer(wnd));
    for (auto mscb : engine->MouseScrollCallbacks)
      mscb(engine, x, y);
  });
  glfwSetFramebufferSizeCallback(window, [](GLFWwindow *wnd, int width, int height) {
    auto engine = static_cast<Engine *>(glfwGetWindowUserPointer(wnd));
    for (auto fbrcb : engine->ResizeCallbacks)
      fbrcb(engine, width, height);
  });
}

void Engine::Shutdown() {
  glfwSetWindowUserPointer(window, NULL);
  if (window)
    glfwDestroyWindow(window);
  glfwTerminate();
}

Engine::~Engine() {
  GWORLD.Destroy();
}

void Engine::Update() {
  ActionQueue.clear(); // clear the action queue
  glfwPollEvents(); // poll the events
  GWORLD.Update();  // call the Update and LateUpdate
}
void Engine::RenderBegin() { GWORLD.RenderBegin(); }
void Engine::RenderEnd() { GWORLD.RenderEnd(); }
void Engine::Start() { 
  // load default assets from the loader
  Loader.LoadDefaultAssets();
  GWORLD.Start();
}
bool Engine::Run() { return !glfwWindowShouldClose(window); }

const int Engine::GetKey(int key) { return glfwGetKey(window, key); }

const int Engine::GetMouseButton(int button) {
  return glfwGetMouseButton(window, button);
}

}; // namespace aEngine
