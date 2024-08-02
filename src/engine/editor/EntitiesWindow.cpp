#include "engine/EditorWindows.hpp"

inline void DrawHierarchyGUI(Entity *entity, ECS::EntityID &selectedEntity,
                             ImGuiTreeNodeFlags nodeFlag) {
  bool isSelected = selectedEntity == entity->ID;
  ImGuiTreeNodeFlags finalFlag = nodeFlag;
  if (isSelected)
    finalFlag |= ImGuiTreeNodeFlags_Selected;
  if (entity->children.size() == 0)
    finalFlag |= ImGuiTreeNodeFlags_Bullet;
  bool nodeOpen = ImGui::TreeNodeEx((void *)(intptr_t)entity->ID, finalFlag,
                                    entity->name.c_str());
  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    selectedEntity = entity->ID;
  // drag drop control
  if (ImGui::BeginDragDropSource()) {
    ImGui::SetDragDropPayload("CHANGE_ENTITY_HIERARCHY", &entity,
                              sizeof(Entity *));
    ImGui::Text("Drag drop to change hierarchy");
    ImGui::EndDragDropSource();
  }
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("CHANGE_ENTITY_HIERARCHY")) {
      Entity *newChild = *(Entity **)payload->Data;
      ECS::EManager.EntityFromID(entity->ID)->AssignChild(newChild);
    }
    ImGui::EndDragDropTarget();
  }
  // right click context menu
  if (ImGui::BeginPopupContextItem(
          (entity->name + std::to_string((unsigned int)entity->ID)).c_str(),
          ImGuiPopupFlags_MouseButtonRight)) {
    ImGui::MenuItem("Entity Options", nullptr, nullptr, false);
    if (ImGui::MenuItem("Remove")) {
      if (entity->children.size() > 0)
        Console.Log("[info]: Destroy entity %s and all its children\n",
                    entity->name.c_str());
      else
        Console.Log("[info]: Destroy entity %s\n", entity->name.c_str());
      ECS::EManager.DestroyEntity(entity->ID);
      selectedEntity = (ECS::EntityID)(-1);
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
  if (nodeOpen) {
    for (auto child : entity->children)
      DrawHierarchyGUI(child, selectedEntity, nodeFlag);
    ImGui::TreePop();
  }
}

void EditorWindows::EntitiesWindow() {
  ImGui::Begin("Entities");
  ImGui::SeparatorText("Scene");
  // Right-click context menu for the parent window
  if (!ImGui::IsAnyItemHovered() &&
      ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) {
    // open the window context menu
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
      ImGui::OpenPopup("EntitiesWindowContextMenu");
    // unselect entities
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
      selectedEntity = (ECS::EntityID)(-1);
  }
  if (ImGui::BeginPopup("EntitiesWindowContextMenu")) {
    ImGui::MenuItem("Window Options", nullptr, nullptr, false);
    if (ImGui::BeginMenu("Create Entity")) {
      ImGui::MenuItem("Entity Types", nullptr, nullptr, false);
      if (ImGui::MenuItem("Null Entity")) {
        ECS::EManager.AddNewEntity();
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Cube")) {
        auto cube = ECS::EManager.AddNewEntity();
        cube->AddComponent<BaseMaterial>();
        cube->AddComponent<MeshRenderer>(
            Core.RManager.GetMesh("::cubePrimitive"));
      }
      if (ImGui::MenuItem("Plane")) {
        auto plane = ECS::EManager.AddNewEntity();
        plane->AddComponent<BaseMaterial>();
        plane->AddComponent<MeshRenderer>(
            Core.RManager.GetMesh("::planePrimitive"));
      }
      if (ImGui::MenuItem("Sphere")) {
        auto sphere = ECS::EManager.AddNewEntity();
        sphere->AddComponent<BaseMaterial>();
        sphere->AddComponent<MeshRenderer>(
            Core.RManager.GetMesh("::spherePrimitive"));
      }
      if (ImGui::MenuItem("Cylinder")) {
        auto cylinder = ECS::EManager.AddNewEntity();
        cylinder->AddComponent<BaseMaterial>();
        cylinder->AddComponent<MeshRenderer>(
            Core.RManager.GetMesh("::cylinderPrimitive"));
      }
      if (ImGui::MenuItem("Cone")) {
        auto cone = ECS::EManager.AddNewEntity();
        cone->AddComponent<BaseMaterial>();
        cone->AddComponent<MeshRenderer>(
            Core.RManager.GetMesh("::conePrimitive"));
      }
      ImGui::EndMenu();
    }
    ImGui::EndPopup();
  }
  ImGui::BeginChild("Entities List", {-1, ImGui::GetContentRegionAvail().y});
  auto entities = ECS::EManager.HierarchyRoots;
  static ImGuiTreeNodeFlags guiTreeNodeFlags =
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
      ImGuiTreeNodeFlags_SpanAvailWidth;
  for (auto i = 0; i < entities.size(); ++i) {
    DrawHierarchyGUI(entities[i], selectedEntity, guiTreeNodeFlags);
  }
  ImGui::EndChild();
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("IMPORT_MODEL_ASSETS")) {
      char *modelPath = (char *)payload->Data;
      // Console.Log(modelPath);
      auto modelEntity = Core.RManager.GetModelEntity(modelPath);
    }
    ImGui::EndDragDropTarget();
  }
  ImGui::End();
}