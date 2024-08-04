#include "ecs/components/Material.hpp"
#include "ecs/components/Lights.hpp"
#include "ecs/components/Lights.hpp"
#include "ecs/components/Camera.hpp"
#include "ecs/components/MeshRenderer.hpp"

#include "ecs/systems/gui/GuiSystem.hpp"

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
      Core.EManager.EntityFromID(entity->ID)->AssignChild(newChild);
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
      Core.EManager.DestroyEntity(entity->ID);
      selectedEntity = (ECS::EntityID)(-1);
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::BeginMenu("Rename")) {
      ImGui::MenuItem("Entity Name", nullptr, nullptr, false);
      ImGui::PushItemWidth(120);
      static char entityNewName[50] = {0};
      ImGui::InputText("##renameentity", entityNewName, sizeof(entityNewName));
      if (ImGui::Button("Confirm")) {
        Core.EManager.EntityFromID(selectedEntity)->name = entityNewName;
        std::strcpy(entityNewName, "");
        ImGui::CloseCurrentPopup();
      }
      ImGui::PopItemWidth();
      ImGui::EndMenu();
    }
    ImGui::EndPopup();
  }
  if (nodeOpen) {
    for (auto child : entity->children)
      DrawHierarchyGUI(child, selectedEntity, nodeFlag);
    ImGui::TreePop();
  }
}

void GuiSystem::EntitiesWindow() {
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
        Core.EManager.AddNewEntity();
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Cube")) {
        auto cube = Core.EManager.AddNewEntity();
        cube->AddComponent<Material>();
        cube->AddComponent<MeshRenderer>(
            Core.RManager.GetMesh("::cubePrimitive"));
      }
      if (ImGui::MenuItem("Plane")) {
        auto plane = Core.EManager.AddNewEntity();
        plane->AddComponent<Material>();
        plane->AddComponent<MeshRenderer>(
            Core.RManager.GetMesh("::planePrimitive"));
      }
      if (ImGui::MenuItem("Sphere")) {
        auto sphere = Core.EManager.AddNewEntity();
        sphere->AddComponent<Material>();
        sphere->AddComponent<MeshRenderer>(
            Core.RManager.GetMesh("::spherePrimitive"));
      }
      if (ImGui::MenuItem("Cylinder")) {
        auto cylinder = Core.EManager.AddNewEntity();
        cylinder->AddComponent<Material>();
        cylinder->AddComponent<MeshRenderer>(
            Core.RManager.GetMesh("::cylinderPrimitive"));
      }
      if (ImGui::MenuItem("Cone")) {
        auto cone = Core.EManager.AddNewEntity();
        cone->AddComponent<Material>();
        cone->AddComponent<MeshRenderer>(
            Core.RManager.GetMesh("::conePrimitive"));
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Camera")) {
        auto camera = Core.EManager.AddNewEntity();
        camera->name = "Camera";
        camera->AddComponent<Camera>();
        Core.SetActiveCamera(camera->ID);
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Directional Light")) {
        auto dLight = Core.EManager.AddNewEntity();
        dLight->name = "Light";
        dLight->SetGlobalRotationDegree(vec3(180.0f, 0.0f, 0.0f));
        dLight->AddComponent<Light>();
        dLight->GetComponent<Light>().Type = Light::LIGHT_TYPE::DIRECTIONAL_LIGHT;
      }
      if (ImGui::MenuItem("Point Light")) {
        auto pLight = Core.EManager.AddNewEntity();
        pLight->name = "Point light";
        pLight->AddComponent<Light>();
        pLight->GetComponent<Light>().Type = Light::LIGHT_TYPE::POINT_LIGHT;
      }
      if (ImGui::MenuItem("Spot Light")) {
        auto sLight = Core.EManager.AddNewEntity();
        sLight->name = "Spot light";
        sLight->AddComponent<Light>();
        sLight->GetComponent<Light>().Type = Light::LIGHT_TYPE::SPOT_LIGHT;
      }
      ImGui::EndMenu();
    }
    ImGui::EndPopup();
  }
  ImGui::BeginChild("Entities List", {-1, ImGui::GetContentRegionAvail().y});
  auto entities = Core.EManager.HierarchyRoots;
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
      Console.Log("[info]: import modelfrom %s\n", modelPath);
      auto modelEntity = Core.RManager.GetModelEntity(modelPath);
    }
    ImGui::EndDragDropTarget();
  }
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("IMPORT_SCENE")) {
      char *scenePath = (char *)payload->Data;
      Core.ReloadScene(scenePath);
    }
    ImGui::EndDragDropTarget();
  }
  ImGui::End();
}