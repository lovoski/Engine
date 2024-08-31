#include "../Editor.hpp"

#include "Component/Animator.hpp"
#include "Component/Camera.hpp"
#include "Component/Light.hpp"
#include "Component/MeshRenderer.hpp"

#include "Function/Render/RenderPass.hpp"
#include "Function/Render/Mesh.hpp"
#include "Function/Render/Shader.hpp"

using glm::vec2;
using glm::vec3;
using glm::vec4;
using std::string;
using std::vector;

inline void DrawHierarchyGUI(Entity *entity, EntityID &selectedEntity,
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
    ImGui::SetDragDropPayload("ENTITYID_DATA", &entity, sizeof(Entity *));
    ImGui::Text("Drag drop to change hierarchy");
    ImGui::EndDragDropSource();
  }
  // reorder the entity hierarchy
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("ENTITYID_DATA")) {
      Entity *newChild = *(Entity **)payload->Data;
      GWORLD.EntityFromID(entity->ID)->AssignChild(newChild);
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
        LOG_F(INFO, "Destroy entity %s and all its children", entity->name.c_str());
      else
        LOG_F(INFO, "Destroy entity %s", entity->name.c_str());
      GWORLD.DestroyEntity(entity->ID);
      // reset selected entity every time remove an entity
      selectedEntity = (EntityID)(-1);
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::BeginMenu("Rename")) {
      ImGui::MenuItem("Entity Name", nullptr, nullptr, false);
      ImGui::PushItemWidth(120);
      static char entityNewName[50] = {0};
      ImGui::InputText("##renameentity", entityNewName, sizeof(entityNewName));
      if (ImGui::Button("Confirm")) {
        GWORLD.EntityFromID(selectedEntity)->name = entityNewName;
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
        GWORLD.AddNewEntity();
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Cube")) {
        auto cube = GWORLD.AddNewEntity();
        cube->name = "Cube";
        cube->AddComponent<MeshRenderer>(Loader.GetMesh("::cubePrimitive", ""));
        cube->GetComponent<MeshRenderer>()->AddPass<Render::Diffuse>(
            nullptr, "Cube Mat");
      }
      if (ImGui::MenuItem("Plane")) {
        auto plane = GWORLD.AddNewEntity();
        plane->name = "Plane";
        plane->AddComponent<MeshRenderer>(
            Loader.GetMesh("::planePrimitive", ""));
        plane->GetComponent<MeshRenderer>()->AddPass<Render::Diffuse>(
            nullptr, "Plane Mat");
      }
      if (ImGui::MenuItem("Sphere")) {
        auto sphere = GWORLD.AddNewEntity();
        sphere->name = "Sphere";
        sphere->AddComponent<MeshRenderer>(
            Loader.GetMesh("::spherePrimitive", ""));
        sphere->GetComponent<MeshRenderer>()->AddPass<Render::Diffuse>(
            nullptr, "Sphere Mat");
      }
      if (ImGui::MenuItem("Cylinder")) {
        auto cylinder = GWORLD.AddNewEntity();
        cylinder->name = "Cylinder";
        cylinder->AddComponent<MeshRenderer>(
            Loader.GetMesh("::cylinderPrimitive", ""));
        cylinder->GetComponent<MeshRenderer>()->AddPass<Render::Diffuse>(
            nullptr, "Cylinder Mat");
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Camera")) {
        auto camera = GWORLD.AddNewEntity();
        camera->name = "Camera";
        camera->AddComponent<Camera>();
        GWORLD.SetActiveCamera(camera->ID);
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Directional Light")) {
        auto dLight = GWORLD.AddNewEntity();
        dLight->name = "Light";
        dLight->SetGlobalRotation(
            glm::quat(glm::radians(vec3(180.0f, 0.0f, 0.0f))));
        dLight->AddComponent<Light>();
        dLight->GetComponent<Light>()->type = LIGHT_TYPE::DIRECTIONAL_LIGHT;
      }
      if (ImGui::MenuItem("Point Light")) {
        auto pLight = GWORLD.AddNewEntity();
        pLight->name = "Point light";
        pLight->AddComponent<Light>();
        pLight->GetComponent<Light>()->type = LIGHT_TYPE::POINT_LIGHT;
      }
      ImGui::EndMenu();
    }
    ImGui::EndPopup();
  }
  ImGui::BeginChild("Entities List", {-1, ImGui::GetContentRegionAvail().y});
  auto entities = GWORLD.HierarchyRoots;
  static ImGuiTreeNodeFlags guiTreeNodeFlags =
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
      ImGuiTreeNodeFlags_SpanAvailWidth;
  for (auto i = 0; i < entities.size(); ++i) {
    DrawHierarchyGUI(entities[i], context.selectedEntity, guiTreeNodeFlags);
  }
  ImGui::EndChild();
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("ASSET_FILENAME")) {
      char *assetFilename = (char *)payload->Data;
      fs::path filename = fs::path(assetFilename);
      std::string extension = filename.extension().string();
      if (extension == ".bvh") {
        // handle bvh mocap import
        auto motion = Loader.GetMotion(filename.string());
        auto parent = GWORLD.AddNewEntity();
        // attach animator component to a proxy entity
        parent->AddComponent<Animator>(motion);
        parent->name = filename.stem().string();
        // set up variables for animator component
        parent->GetComponent<Animator>()->ShowSkeleton = true;
        // make skeleton hierarchy a child of proxy entity
        auto skelEntity = parent->GetComponent<Animator>()->skeleton;
        parent->children.push_back(skelEntity);
        skelEntity->parent = parent.get();
      } else if (extension == ".obj" || extension == ".off") {
        // handle plane model import
        auto modelMeshes = Loader.GetModel(filename.string());
        auto parentEntity = GWORLD.AddNewEntity();
        parentEntity->name = filename.stem().string();
        // create one unified material for the whole object
        auto unifyMat = Loader.InstantiateMaterial<Render::Diffuse>(
            filename.stem().string());
        for (auto cmesh : modelMeshes) {
          auto childEntity = GWORLD.AddNewEntity();
          childEntity->name = cmesh->identifier;
          childEntity->AddComponent<MeshRenderer>(cmesh);
          childEntity->GetComponent<MeshRenderer>()->AddPass(unifyMat, unifyMat->identifier);
          // setup parent child relation
          parentEntity->AssignChild(childEntity.get());
        }
      } else if (extension == ".fbx") {
        // fbx model possibly contains animation data
        auto modelMeshes = Loader.LoadAndCreateEntityFromFile(filename.string());
      }
      // TODO:
      // Core.ReloadScene(scenePath);
    }
    ImGui::EndDragDropTarget();
  }
  ImGui::End();
}