#pragma once

#include "Engine.hpp"
#include "Events.hpp"
#include "ecs/components/Camera.hpp"
#include "ecs/components/Material.hpp"
#include "ecs/components/MeshRenderer.hpp"
#include "ecs/systems/render/FrameBuffer.hpp"
#include "global.hpp"
#include "utils/Math.hpp"

class EditorWindows {
public:
  EditorWindows() {}
  ~EditorWindows() {}

  EditorWindows(const EditorWindows &) = delete;
  const EditorWindows &operator=(const EditorWindows &) = delete;

  static EditorWindows &Ref() {
    static EditorWindows reference;
    return reference;
  }

  void Initialize();

  void Destroy();

  void MainMenuBar();
  void EntitiesWindow();
  void ConsoleWindow();
  void AssetsWindow();
  void ComponentsWindow();

  // Define the gui layout, returns the available size for scene rendering
  void RenderStart(Graphics::FrameBuffer *sceneBuffer);
  void RenderComplete();

  // Get the active camera in the scene
  bool GetActiveCamera(ECS::EntityID &camera);

  // The camera entity must has a transform component and a camera component
  bool SetActiveCamera(ECS::EntityID camera);

  bool InSceneWindow(float x, float y) {
    return x >= SceneWindowPos.x && x <= SceneWindowPos.x + SceneWindowSize.x &&
           y >= SceneWindowPos.y && y <= SceneWindowPos.y + SceneWindowSize.y;
  }

  bool LoopCursorInSceneWindow();

  // export imgui io to receive events
  ImGuiIO *io = nullptr;

  vec2 SceneWindowSize = vec2(0.0f);
  vec2 SceneWindowPos = vec2(0.0f);

private:
  const char *layoutFileName = "layout.ini";

  bool hasActiveCamera = false;
  ECS::EntityID activeCamera;

  ECS::EntityID selectedEntity = -1;
};

static EditorWindows &EditorContext = EditorWindows::Ref();
