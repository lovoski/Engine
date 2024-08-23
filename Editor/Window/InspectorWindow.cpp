#include "../Editor.hpp"
#include <limits>

#include "Component/Animator.hpp"
#include "Component/Camera.hpp"
#include "Component/Light.hpp"
#include "Component/MeshRenderer.hpp"
#include "Component/NativeScript.hpp"

#include "Function/Render/MaterialData.hpp"
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
  if (ImGui::TreeNode("Transform")) {
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
    ImGui::TreePop();
  }
}

void Editor::InspectorWindow() {
  ImGui::Begin("Components");
  // // Right-click context menu for the parent window
  // if (!ImGui::IsAnyItemHovered() &&
  //     ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) {
  //   // open the window context menu
  //   if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
  //     ImGui::OpenPopup("ComponentWindowContextMenu");
  //   // unselect entities
  //   // if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
  //   //   selectedEntity = (ECS::EntityID)(-1);
  // }
  // if (selectedEntity != (ECS::EntityID)(-1) &&
  // ImGui::BeginPopup("ComponentWindowContextMenu")) {
  //   ImGui::MenuItem("Window Options", nullptr, nullptr, false);
  //   ImGui::EndPopup();
  // }
  static bool pannelLocked = false;
  ImGui::Checkbox("Lock Panel", &pannelLocked);
  if (!pannelLocked) {
    context.lockedSelectedEntity = context.selectedEntity;
  }
  GWORLD.EntityValid(context.lockedSelectedEntity);
  EntityID entity =
      pannelLocked ? context.lockedSelectedEntity : context.selectedEntity;
  // the entity must be valid
  if (entity != (EntityID)(-1)) {
    string entityName =
        "Active Entity : " + GWORLD.EntityFromID(entity)->name;
    ImGui::SeparatorText(entityName.c_str());
    ImGui::BeginChild("Components List",
                      {-1, ImGui::GetContentRegionAvail().y});
    DrawTransformGUI(entity);
    if (GWORLD.HasComponent<Camera>(entity))
      GWORLD.GetComponent<Camera>(entity).DrawInspectorGUI();
    if (GWORLD.HasComponent<Light>(entity))
      GWORLD.GetComponent<Light>(entity).DrawInspectorGUI();
    if (GWORLD.HasComponent<Animator>(entity))
      GWORLD.GetComponent<Animator>(entity).DrawInspectorGUI();
    if (GWORLD.HasComponent<NativeScript>(entity))
      GWORLD.GetComponent<NativeScript>(entity).DrawInspectorGUI();
    if (GWORLD.HasComponent<MeshRenderer>(entity))
      GWORLD.GetComponent<MeshRenderer>(entity).DrawInspectorGUI();
    ImGui::EndChild();
  }
  ImGui::End();
}
