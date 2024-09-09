#include "Scripts/Animation/SAMERetarget.hpp"
#include "Scripts/Animation/VisMetrics.hpp"

#include "../Editor.hpp"
#include <limits>

#include "Component/Animator.hpp"
#include "Component/Camera.hpp"
#include "Component/DeformRenderer.hpp"
#include "Component/Light.hpp"
#include "Component/MeshRenderer.hpp"
#include "Component/NativeScript.hpp"
#include "Component/Mesh.hpp"

#include "Function/Render/RenderPass.hpp"
#include "Function/Render/Shader.hpp"

using glm::quat;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using std::string;
using std::vector;

#define MAX_FLOAT std::numeric_limits<float>::max()

inline void DrawTransformGUI(EntityID selectedEntity) {
  auto transform = GWORLD.EntityFromID(selectedEntity);
  if (ImGui::CollapsingHeader("Transform")) {
    ImGui::MenuItem("Global Properties", nullptr, nullptr, false);
    vec3 position = transform->Position();
    vec3 scale = transform->Scale();
    // dirctly decompose to euler angles could result in gimbal lock
    // glm::mat3 rotation = glm::mat3_cast(transform->Rotation());
    // vec3 angles = glm::degrees(transform->EulerAngles());
    // printf("x=%f, y=%f, z=%f\n", angles.x, angles.y, angles.z);
    float positions[3] = {position.x, position.y, position.z};
    float scales[3] = {scale.x, scale.y, scale.z};
    // float rotations[3] = {0.0f};
    if (ImGui::DragFloat3("Position", positions, 0.01f, -MAX_FLOAT, MAX_FLOAT))
      transform->SetGlobalPosition(
          vec3(positions[0], positions[1], positions[2]));
    // if (ImGui::DragFloat3("Rotation", rotations, 1.0f, 360.0f)) {
    //   transform->SetGlobalRotation(
    //       quat(glm::radians(vec3(rotations[0], rotations[1],
    //       rotations[2]))));
    // }
    if (ImGui::DragFloat3("Scale", scales, 0.01f, 0.0f, MAX_FLOAT))
      transform->SetGlobalScale(vec3(scales[0], scales[1], scales[2]));
  }
}

template <typename T> void DrawComponent(EntityID entity, std::string name) {
  static bool showComponent = false;
  bool hasComponent = GWORLD.HasComponent<T>(entity);
  if (hasComponent) {
    showComponent = true;
  } else {
    showComponent = false;
  }
  if (ImGui::CollapsingHeader(name.c_str(), &showComponent)) {
    GWORLD.GetComponent<T>(entity)->DrawInspectorGUI();
  }
  if (hasComponent && !showComponent) {
    // the component should be removed
    if constexpr (std::is_same_v<Animator, T>) {
      // animator is not a removable component
      // there might be some component from other entities depending on it
      LOG_F(WARNING, "can't remove animator from entity %d", entity);
    } else {
      LOG_F(INFO, "remove component %s from entiy %d", name.c_str(), entity);
      GWORLD.RemoveComponent<T>(entity);
    }
  }
}

void InspectorRightClickMenu(EntityID entity) {
  if (ImGui::BeginPopup("ComponentWindowContextMenu")) {
    if (ImGui::MenuItem("Add Component", nullptr, nullptr, false))
      ;
    ImGui::Separator();
    if (ImGui::MenuItem("Native Script")) {
      GWORLD.AddComponent<NativeScript>(entity);
    }
    if (ImGui::MenuItem("Geometry Mesh"))
      GWORLD.AddComponent<Mesh>(entity, nullptr);
    ImGui::EndPopup();
  }
}

void Editor::InspectorWindow() {
  ImGui::Begin("Components", &showInspectorWindow);
  // Right-click context menu for the parent window
  if (!ImGui::IsAnyItemHovered() &&
      ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) {
    // open the window context menu
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
      ImGui::OpenPopup("ComponentWindowContextMenu");
  }
  static bool pannelLocked = false;
  ImGui::Checkbox("Lock Panel", &pannelLocked);
  if (!pannelLocked) {
    context.lockedSelectedEntity = context.selectedEntity;
  }
  GWORLD.EntityValid(context.lockedSelectedEntity);
  EntityID entity =
      pannelLocked ? context.lockedSelectedEntity : context.selectedEntity;
  // the entity must be valid
  if (GWORLD.EntityValid(entity)) {
    InspectorRightClickMenu(
        entity); // only show the menu when the component is valid
    string entityName = "Active Entity : " + std::to_string(entity);
    ImGui::SeparatorText(entityName.c_str());
    ImGui::BeginChild("Components List",
                      {-1, ImGui::GetContentRegionAvail().y});
    DrawTransformGUI(entity);

    // draw components of this entity if exists
    // new components need to manually register here
    // TODO: automatically register component?
    DrawComponent<Mesh>(entity, "Mesh Data");
    DrawComponent<Camera>(entity, "Camera");
    DrawComponent<Light>(entity, "Light");
    DrawComponent<Animator>(entity, "Animator");
    DrawComponent<MeshRenderer>(entity, "Mesh Renderer");
    DrawComponent<DeformRenderer>(entity, "Deform Renderer");
    DrawComponent<NativeScript>(entity, "Native Script");
    ImGui::EndChild();
  }
  ImGui::End();
}
