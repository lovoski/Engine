#pragma once

#include "global.hpp"
#include "ecs/ecs.hpp"

#include "resource/ResourceManager.hpp"

const int WIDTH = 1920, HEIGHT = 1080;

class Engine {
public:
  Engine(const Engine &) = delete;
  ~Engine();
  // Remove this operator
  // Making sure there is only one instance of the engine class
  Engine &operator=(const Engine &) = delete;

  static Engine &Ref() {
    static Engine reference;
    return reference;
  }

  void Quit();
  void Update();
  void Initialize();

  const bool Run() const { return run; }
  GLFWwindow &Window() { return *window; }
  const float VideoWdith() const { return videoWidth; }
  const float VideoHeight() const { return videoHeight; }

  Resource::ResourceManager RManager;

private:
  Engine();
  bool run;
  GLFWwindow *window;
  float videoWidth, videoHeight;
};

static Engine &Core = Engine::Ref();