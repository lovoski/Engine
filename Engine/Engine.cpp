#include "Engine.hpp"

namespace aEngine {

Engine::Engine(int width, int height)
    : windowWidth(width), windowHeight(height) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
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

  // create the default scene with the same size as the window
  scene = new Scene(width, height);
  // pass the pointer of current windows to context
  scene->Context.window = window;

  // setup the user pointer so we can register events
  glfwSetWindowUserPointer(window, this);

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
  if (scene) {
    scene->Destroy();
    delete scene;
  }
}

void Engine::Update() {
  glfwPollEvents(); // poll the events
  scene->Update();  // call the Update and LateUpdate
}
void Engine::RenderBegin() { scene->RenderBegin(); }
void Engine::RenderEnd() { scene->RenderEnd(); }
void Engine::Start() { 
  // load default assets from the loader
  Loader.LoadDefaultAssets();
  scene->Start();
}
bool Engine::Run() { return !glfwWindowShouldClose(window); }

const int Engine::GetKey(int key) { return glfwGetKey(window, key); }

const int Engine::GetMouseButton(int button) {
  return glfwGetMouseButton(window, button);
}

}; // namespace aEngine
