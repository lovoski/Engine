/**
 * Engine is a helper class to maintain the scenes
 * while Editor provides GUI access to the api provided by Engine.
 * 
 * Never use `GWORLD` singletom itself to create application.
 */
#pragma once

#include "Scene.hpp"
#include "Utils/AssetsLoader.hpp"

namespace aEngine {

// TODO: the event system
enum ACTION_TYPE {
  MOUSE_SCROLL
};

struct Action {
  ACTION_TYPE type;
  void *payload;
};

class Engine {
public:
  Engine(int width, int height);
  Engine(const Engine &) = delete;
  const Engine &operator=(const Engine &) = delete;
  ~Engine();

  // Update all registered systems
  void Update();
  // Render the scene in a framebuffer
  void RenderBegin();
  // Swaps the front and back buffer
  void RenderEnd();

  void Start();
  bool Run();

  void Shutdown();

  const int GetKey(int key);
  const int GetMouseButton(int button);

  std::vector<std::function<void(Engine *, double, double)>> MouseMoveCallbacks;
  std::vector<std::function<void(Engine *)>> WindowCloseCallbacks;
  std::vector<std::function<void(Engine *, double, double)>> MouseScrollCallbacks;
  std::vector<std::function<void(Engine *, int, int)>> ResizeCallbacks;

  std::vector<Action> ActionQueue;

  glm::vec2 _mouseScrollOffsets;

private:
  int windowWidth, windowHeight;

  GLFWwindow *window;
};

}; // namespace aEngine
