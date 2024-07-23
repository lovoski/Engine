#include "engine.hpp"

Engine::Engine() : run(true), window(NULL), videoWidth(WIDTH), videoHeight(HEIGHT) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  auto &monitor = *glfwGetVideoMode(glfwGetPrimaryMonitor());
  glfwWindowHint(GLFW_RED_BITS, monitor.redBits);
  glfwWindowHint(GLFW_BLUE_BITS, monitor.blueBits);
  glfwWindowHint(GLFW_GREEN_BITS, monitor.greenBits);
  glfwWindowHint(GLFW_REFRESH_RATE, monitor.refreshRate);

  window = glfwCreateWindow(videoWidth, videoHeight, "Engine", NULL, NULL);
  assert(window && "Failed to create GLFW window");
  glfwMakeContextCurrent(window);

  assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) && "Failed to load GLAD");
}

Engine::~Engine() {
  glfwTerminate();
}

void Engine::Initialize() {}

void Engine::Update() {}

void Engine::Quit() {
  glfwSetWindowShouldClose(window, GLFW_TRUE);
  run = false;
}