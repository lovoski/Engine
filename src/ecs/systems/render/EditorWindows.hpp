#pragma once

#include "FrameBuffer.hpp"
#include "basics.hpp"
#include "ecs/components/Camera.hpp"
#include "ecs/components/Material.hpp"
#include "ecs/components/MeshRenderer.hpp"
#include "global.hpp"

class EditorWindows {
public:
  EditorWindows() {}
  ~EditorWindows() {}

  void Initialize();

  void Destroy();

  void MainMenuBar();
  void EntitiesWindow();
  void ConsoleWindow();
  void AssetsWindow();
  void ComponentsWindow();

  // Define the gui layout, returns the available size for scene rendering
  vec2 RenderStart(Graphics::FrameBuffer *sceneBuffer);
  void RenderComplete();

  // Get the active camera in the scene
  bool GetActiveCamera(ECS::EntityID &camera) {
    if (hasActiveCamera) {
      camera = activeCamera;
      return true;
    } else {
      cout << "There's no active camera" << endl;
      camera = (ECS::EntityID)(-1);
      return false;
    }
  }

  // The camera entity must has a transform component and a camera component
  bool SetActiveCamera(ECS::EntityID camera) {
    if (ECS::Manager.HasComponent<Camera>(camera) &&
        ECS::Manager.HasComponent<Transform>(camera)) {
      hasActiveCamera = true;
      activeCamera = camera;
      return true;
    } else {
      cout << "Not a valid camera entity" << endl;
      // there could exist an active camera,
      // don't reset the hasActiveCamera flag
      return false;
    }
  }

private:
  const char *layoutFileName = "layout.ini";

  bool hasActiveCamera = false;
  ECS::EntityID activeCamera;

  bool hasSelectedEntity = false;
  ECS::EntityID selectedEntity;
};