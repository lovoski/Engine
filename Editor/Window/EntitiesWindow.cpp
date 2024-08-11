#include "../Editor.hpp"

#include "Component/Camera.hpp"
#include "Component/Material.hpp"
#include "Component/MeshRenderer.hpp"
#include "Component/Light.hpp"

#include "Utils/Render/Mesh.hpp"
#include "Utils/Render/Shader.hpp"
#include "Utils/Render/MaterialData.hpp"

using glm::vec2;
using glm::vec3;
using glm::vec4;
using std::string;
using std::vector;

inline void DrawHierarchyGUI(Entity *entity, EntityID &selectedEntity,
                             ImGuiTreeNodeFlags nodeFlag, Engine *engine) {
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
      engine->GetScene()->EntityFromID(entity->ID)->AssignChild(newChild);
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
      engine->GetScene()->DestroyEntity(entity->ID);
      selectedEntity = (EntityID)(-1);
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::BeginMenu("Rename")) {
      ImGui::MenuItem("Entity Name", nullptr, nullptr, false);
      ImGui::PushItemWidth(120);
      static char entityNewName[50] = {0};
      ImGui::InputText("##renameentity", entityNewName, sizeof(entityNewName));
      if (ImGui::Button("Confirm")) {
        engine->GetScene()->EntityFromID(selectedEntity)->name = entityNewName;
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
      DrawHierarchyGUI(child, selectedEntity, nodeFlag, engine);
    ImGui::TreePop();
  }
}

void Editor::EntitiesWindow() {
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
      context.selectedEntity = (EntityID)(-1);
  }
  if (ImGui::BeginPopup("EntitiesWindowContextMenu")) {
    ImGui::MenuItem("Window Options", nullptr, nullptr, false);
    if (ImGui::BeginMenu("Create Entity")) {
      ImGui::MenuItem("Entity Types", nullptr, nullptr, false);
      if (ImGui::MenuItem("Null Entity")) {
        engine->GetScene()->AddNewEntity();
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Cube")) {
        auto cube = engine->GetScene()->AddNewEntity();
        cube->name = "Cube";
        cube->AddComponent<Material>();
        cube->AddComponent<MeshRenderer>(
            Loader.GetMesh("::cubePrimitive", ""));
      }
      if (ImGui::MenuItem("Plane")) {
        auto plane = engine->GetScene()->AddNewEntity();
        plane->name = "Plane";
        plane->AddComponent<Material>();
        plane->AddComponent<MeshRenderer>(
            Loader.GetMesh("::planePrimitive", ""));
      }
      if (ImGui::MenuItem("Sphere")) {
        auto sphere = engine->GetScene()->AddNewEntity();
        sphere->name = "Sphere";
        sphere->AddComponent<Material>();
        sphere->AddComponent<MeshRenderer>(
            Loader.GetMesh("::spherePrimitive", ""));
      }
      if (ImGui::MenuItem("Cylinder")) {
        auto cylinder = engine->GetScene()->AddNewEntity();
        cylinder->name = "Cylinder";
        cylinder->AddComponent<Material>();
        cylinder->AddComponent<MeshRenderer>(
            Loader.GetMesh("::cylinderPrimitive", ""));
      }
      if (ImGui::MenuItem("Cone")) {
        auto cone = engine->GetScene()->AddNewEntity();
        cone->name = "Cone";
        cone->AddComponent<Material>();
        cone->AddComponent<MeshRenderer>(
            Loader.GetMesh("::conePrimitive", ""));
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Camera")) {
        auto camera = engine->GetScene()->AddNewEntity();
        camera->name = "Camera";
        camera->AddComponent<Camera>();
        engine->GetScene()->SetActiveCamera(camera->ID);
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Directional Light")) {
        auto dLight = engine->GetScene()->AddNewEntity();
        dLight->name = "Light";
        dLight->SetGlobalRotationDegree(vec3(180.0f, 0.0f, 0.0f));
        dLight->AddComponent<Light>();
        dLight->GetComponent<Light>().type = LIGHT_TYPE::DIRECTIONAL_LIGHT;
      }
      if (ImGui::MenuItem("Point Light")) {
        auto pLight = engine->GetScene()->AddNewEntity();
        pLight->name = "Point light";
        pLight->AddComponent<Light>();
        pLight->GetComponent<Light>().type = LIGHT_TYPE::POINT_LIGHT;
      }
      if (ImGui::MenuItem("Spot Light")) {
        auto sLight = engine->GetScene()->AddNewEntity();
        sLight->name = "Spot light";
        sLight->AddComponent<Light>();
        sLight->GetComponent<Light>().type = LIGHT_TYPE::SPOT_LIGHT;
      }
      ImGui::EndMenu();
    }
    ImGui::EndPopup();
  }
  ImGui::BeginChild("Entities List", {-1, ImGui::GetContentRegionAvail().y});
  auto entities = engine->GetScene()->HierarchyRoots;
  static ImGuiTreeNodeFlags guiTreeNodeFlags =
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
      ImGuiTreeNodeFlags_SpanAvailWidth;
  for (auto i = 0; i < entities.size(); ++i) {
    DrawHierarchyGUI(entities[i], context.selectedEntity, guiTreeNodeFlags, engine);
  }
  ImGui::EndChild();
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("IMPORT_MODEL_ASSETS")) {
      char *modelPath = (char *)payload->Data;
      Console.Log("[info]: import modelfrom %s\n", modelPath);
      auto modelMeshes = Loader.GetModel(modelPath);
      auto parentEntity = engine->GetScene()->AddNewEntity();
      parentEntity->name = fs::path(modelPath).stem().string();
      for (auto cmesh : modelMeshes) {
        auto childEntity = engine->GetScene()->AddNewEntity();
        childEntity->name = cmesh->identifier;
        childEntity->AddComponent<Material>();
        childEntity->AddComponent<MeshRenderer>(cmesh);
        // setup parent child relation
        parentEntity->AssignChild(childEntity);
      }
      // auto modelEntity = Loader.GetModelEntity(modelPath);
    }
    ImGui::EndDragDropTarget();
  }
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("IMPORT_SCENE")) {
      char *scenePath = (char *)payload->Data;
      // TODO: 
      // Core.ReloadScene(scenePath);
    }
    ImGui::EndDragDropTarget();
  }
  ImGui::End();
}