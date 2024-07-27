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
  void ComponentsWindow() {
    ImGui::Begin("Components");
    if (ImGui::Button("Add Component", {-1, 40})) {
      Console.Log("add component\n");
    }
    if (selectedEntity != (ECS::EntityID)(-1)) {
      string entityName =
          "Active Entity : " + ECS::Manager.EntityFromID(selectedEntity)->name;
      ImGui::SeparatorText(entityName.c_str());
      if (ECS::Manager.HasComponent<Transform>(selectedEntity)) {
        auto &transform = ECS::Manager.GetComponent<Transform>(selectedEntity);
        if (ImGui::TreeNode("Transform")) {
          ImGui::SeparatorText("Position");
          ImGui::DragFloat(" :pos.X", &transform.Position.x, 0.001f, -MAX_FLOAT,
                           MAX_FLOAT);
          ImGui::DragFloat(" :pos.Y", &transform.Position.y, 0.001f, -MAX_FLOAT,
                           MAX_FLOAT);
          ImGui::DragFloat(" :pos.Z", &transform.Position.z, 0.001f, -MAX_FLOAT,
                           MAX_FLOAT);

          ImGui::SeparatorText("Scale");
          ImGui::DragFloat(" :scale.X", &transform.Scale.x, 0.001f, -MAX_FLOAT,
                           MAX_FLOAT);
          ImGui::DragFloat(" :scale.Y", &transform.Scale.y, 0.001f, -MAX_FLOAT,
                           MAX_FLOAT);
          ImGui::DragFloat(" :scale.Z", &transform.Scale.z, 0.001f, -MAX_FLOAT,
                           MAX_FLOAT);

          auto &rot = transform.Rotation;
          vec3 euler = glm::eulerAngles(rot);
          ImGui::SeparatorText("Rotation");
          bool updateRotation = false;
          updateRotation |= ImGui::DragFloat(" :rot.X", &euler.x, 0.001f,
                                             -MAX_FLOAT, MAX_FLOAT);
          updateRotation |= ImGui::DragFloat(" :rot.Y", &euler.y, 0.001f,
                                             -MAX_FLOAT, MAX_FLOAT);
          updateRotation |= ImGui::DragFloat(" :rot.Z", &euler.z, 0.001f,
                                             -MAX_FLOAT, MAX_FLOAT);
          if (updateRotation)
            rot = quat(euler);
          ImGui::TreePop();
        }
      }
      if (ECS::Manager.HasComponent<Camera>(selectedEntity)) {
        // Console.Log("render camera component\n");
      }
    }
    ImGui::End();
  }

  // Define the gui layout, returns the available size for scene rendering
  void RenderStart(Graphics::FrameBuffer *sceneBuffer);
  void RenderComplete();

  // Get the active camera in the scene
  bool GetActiveCamera(ECS::EntityID &camera) {
    if (hasActiveCamera) {
      camera = activeCamera;
      return true;
    } else {
      Console.Log("[Info]: There's no active camera\n");
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
      Console.Log("[error]: Not a valid camera entity: %ld\n", camera);
      // there could exist an active camera,
      // don't reset the hasActiveCamera flag
      return false;
    }
  }

  bool InSceneWindow(float x, float y) {
    return x >= SceneWindowPos.x && x <= SceneWindowPos.x + SceneWindowSize.x &&
           y >= SceneWindowPos.y && y <= SceneWindowPos.y + SceneWindowSize.y;
  }

  bool LoopCursorInSceneWindow() {
    vec2 cursorPos = Event.MouseCurrentPosition;
    if (!InSceneWindow(cursorPos.x, cursorPos.y)) {
      cursorPos -= SceneWindowPos;
      while (cursorPos.x < 0.0f)
        cursorPos.x += SceneWindowSize.x;
      while (cursorPos.x > SceneWindowSize.x)
        cursorPos.x -= SceneWindowSize.x;
      while (cursorPos.y < 0.0f)
        cursorPos.y += SceneWindowSize.y;
      while (cursorPos.y > SceneWindowSize.y)
        cursorPos.y -= SceneWindowSize.y;
      cursorPos += SceneWindowPos;
      glfwSetCursorPos(&Core.Window(), cursorPos.x, cursorPos.y);
      return false;
    } else
      return true;
  }

  // export imgui io to receive events
  ImGuiIO *io = nullptr;

  vec2 SceneWindowSize = vec2(0.0f);
  vec2 SceneWindowPos = vec2(0.0f);

private:
  const char *layoutFileName = "layout.ini";

  bool hasActiveCamera = false;
  ECS::EntityID activeCamera;

  std::size_t selectedEntityInd = -1;
  ECS::EntityID selectedEntity = -1;
};

static EditorWindows &EditorContext = EditorWindows::Ref();
