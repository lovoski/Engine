#include "engine.hpp"
#include "ecs/systems/render/MeshRendererSystem.hpp"

// the actual implementation of stb image
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Engine::Engine()
    : run(true), window(NULL), videoWidth(WIDTH), videoHeight(HEIGHT) {
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

  assert(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) &&
         "Failed to load GLAD");
}

Engine::~Engine() {
  ECS::Manager.Destroy();
  glfwTerminate();
}

void Engine::Initialize() {
  // register all the systems
  ECS::Manager.RegisterSystem<MeshRendererSystem>();
  // start all the systems
  ECS::Manager.Start();
}

void Engine::Update() { ECS::Manager.Update(); }

void Engine::Quit() {
  glfwSetWindowShouldClose(window, GLFW_TRUE);
  run = false;
}