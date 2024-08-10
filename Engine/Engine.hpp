/**
 * Engine is a helper class to maintain the scenes
 * while Editor provides GUI access to the api provided by Engine
 */
#pragma once

#include "Scene.hpp"
#include "Utils/AssetsLoader.hpp"

namespace aEngine {

class Engine {
public:
  Engine(int width, int height);
  Engine(const Engine &) = delete;
  const Engine &operator=(const Engine &) = delete;
  ~Engine() {}

  void Update();
  void RenderBegin();
  void RenderEnd();

  void Start();
  bool Run();
  void Quit();

  const FrameBuffer *GetSceneFrameBuffer() {
    return scene->Context.frameBuffer;
  }

private:
  Scene *scene;

  bool run;
  int windowWidth, windowHeight;

  GLFWwindow *window;
};

}; // namespace aEngine
