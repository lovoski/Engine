#include "Engine.hpp"
#include "ecs/systems/render/RenderSystem.hpp"
#include "ecs/systems/camera/CameraSystem.hpp"
#include "ecs/systems/light/LightSystem.hpp"

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
  if (!window) {
    cout << "Failed to create GLFW window" << endl;
    return;
  }
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    cout << "Failed to load GLAD" << endl;
    return;
  }
}

Engine::~Engine() {
  ECS::EManager.Destroy();
  glfwTerminate();
}

void Engine::Initialize() {
  RManager.Initialize();

  // register all the systems
  ECS::EManager.RegisterSystem<BaseLightSystem>();
  ECS::EManager.RegisterSystem<RenderSystem>();
  ECS::EManager.RegisterSystem<CameraSystem>();
  // start all the systems
  ECS::EManager.Start();
}

void Engine::Update() { ECS::EManager.Update(); }

void Engine::Quit() {
  glfwSetWindowShouldClose(window, GLFW_TRUE);
  run = false;
}