#include "Engine.hpp"

namespace aEngine {

Engine::Engine(int width, int height) : windowWidth(width), windowHeight(height), run(true) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  auto &monitor = *glfwGetVideoMode(glfwGetPrimaryMonitor());
  glfwWindowHint(GLFW_RED_BITS, monitor.redBits);
  glfwWindowHint(GLFW_BLUE_BITS, monitor.blueBits);
  glfwWindowHint(GLFW_GREEN_BITS, monitor.greenBits);
  glfwWindowHint(GLFW_REFRESH_RATE, monitor.refreshRate);

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

  // load default assets from the loader
  Loader.LoadDefaultAssets();
}

void Engine::Update() {
  glfwPollEvents();
  scene->Update();
}
void Engine::RenderBegin() {
  scene->RenderBegin();
}
void Engine::RenderEnd() {
  scene->RenderEnd();
}
void Engine::Start() {
  scene->Start();
}
bool Engine::Run() {
  return run;
}
void Engine::Quit() {
  glfwSetWindowShouldClose(window, GLFW_TRUE);
  run = false;
}

};
