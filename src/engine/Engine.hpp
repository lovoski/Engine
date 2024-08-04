#pragma once

#include "global.hpp"

#include "ecs/ecs.hpp"
#include "resource/ResourceManager.hpp"

#include "ecs/systems/render/FrameBuffer.hpp"

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
  void Reset();
  void Update();
  void Initialize();

  const bool Run() const { return run; }
  GLFWwindow &Window() { return *window; }
  const float VideoWdith() const { return videoWidth; }
  const float VideoHeight() const { return videoHeight; }

  // Get the active camera in the scene
  bool GetActiveCamera(ECS::EntityID &camera);
  // The camera entity must has a transform component and a camera component
  bool SetActiveCamera(ECS::EntityID camera);

  // Loop the coursor in the scene window range, returns whether the cursor
  // leaves the scene window at this frame or not
  bool LoopCursorInSceneWindow();

  bool InSceneWindow(float x, float y) {
    return x >= SceneWindowPos.x && x <= SceneWindowPos.x + SceneWindowSize.x &&
           y >= SceneWindowPos.y && y <= SceneWindowPos.y + SceneWindowSize.y;
  }

  void ReloadScene(string path);

  ECS::EntityManager EManager;
  Resource::ResourceManager RManager;
  Graphics::FrameBuffer *SceneBuffer;
  glm::vec<2, int> SceneWindowSize;
  glm::vec<2, int> SceneWindowPos;
  string ActiveSceneFile = "::defaultScene";

  Json DumpSceneAsJson();
  void LoadSceneFromJson(Json sceneFileContent);

private:
  Engine();
  bool run;
  GLFWwindow *window;
  float videoWidth, videoHeight;

  // context variables related to rendering
  bool hasActiveCamera = false;
  ECS::EntityID activeCamera;

  // context variables related to scene
  bool reloadScene = false;

};

static Engine &Core = Engine::Ref();