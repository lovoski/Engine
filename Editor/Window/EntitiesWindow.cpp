#include "../Editor.hpp"

#include "Component/Camera.hpp"
#include "Component/Material.hpp"
#include "Component/MeshRenderer.hpp"
#include "Component/Light.hpp"
#include "Component/Animator.hpp"

#include "Utils/Render/Mesh.hpp"
#include "Utils/Render/Shader.hpp"
#include "Utils/Render/MaterialData.hpp"

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
    ImGui::SetDragDropPayload("CHANGE_ENTITY_HIERARCHY", &entity,
                              sizeof(Entity *));
    ImGui::Text("Drag drop to change hierarchy");
    ImGui::EndDragDropSource();
  }
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("CHANGE_ENTITY_HIERARCHY")) {
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
        Console.Log("[info]: Destroy entity %s and all its children\n",
                    entity->name.c_str());
      else
        Console.Log("[info]: Destroy entity %s\n", entity->name.c_str());
      GWORLD.DestroyEntity(entity->ID);
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

void CreateBVHSkeletonHierarchy(Animation::Skeleton *skel, int currentJoint, glm::vec3 parentPos, Entity *parent) {
  for (auto c : skel->jointChildren[currentJoint]) {
    auto ce = GWORLD.AddNewEntity();
    ce->name = skel->jointNames[c];
    ce->SetGlobalPosition(parentPos + skel->jointOffset[c]);
    parent->AssignChild(ce);
    CreateBVHSkeletonHierarchy(skel, c, ce->Position(), ce);
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
        cube->AddComponent<Material>();
        cube->AddComponent<MeshRenderer>(
            Loader.GetMesh("::cubePrimitive", ""));
      }
      if (ImGui::MenuItem("Plane")) {
        auto plane = GWORLD.AddNewEntity();
        plane->name = "Plane";
        plane->AddComponent<Material>();
        plane->AddComponent<MeshRenderer>(
            Loader.GetMesh("::planePrimitive", ""));
      }
      if (ImGui::MenuItem("Sphere")) {
        auto sphere = GWORLD.AddNewEntity();
        sphere->name = "Sphere";
        sphere->AddComponent<Material>();
        sphere->AddComponent<MeshRenderer>(
            Loader.GetMesh("::spherePrimitive", ""));
      }
      if (ImGui::MenuItem("Cylinder")) {
        auto cylinder = GWORLD.AddNewEntity();
        cylinder->name = "Cylinder";
        cylinder->AddComponent<Material>();
        cylinder->AddComponent<MeshRenderer>(
            Loader.GetMesh("::cylinderPrimitive", ""));
      }
      if (ImGui::MenuItem("Cone")) {
        auto cone = GWORLD.AddNewEntity();
        cone->name = "Cone";
        cone->AddComponent<Material>();
        cone->AddComponent<MeshRenderer>(
            Loader.GetMesh("::conePrimitive", ""));
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
        dLight->SetGlobalRotation(glm::quat(glm::radians(vec3(180.0f, 0.0f, 0.0f))));
        dLight->AddComponent<Light>();
        dLight->GetComponent<Light>().type = LIGHT_TYPE::DIRECTIONAL_LIGHT;
      }
      if (ImGui::MenuItem("Point Light")) {
        auto pLight = GWORLD.AddNewEntity();
        pLight->name = "Point light";
        pLight->AddComponent<Light>();
        pLight->GetComponent<Light>().type = LIGHT_TYPE::POINT_LIGHT;
      }
      if (ImGui::MenuItem("Spot Light")) {
        auto sLight = GWORLD.AddNewEntity();
        sLight->name = "Spot light";
        sLight->AddComponent<Light>();
        sLight->GetComponent<Light>().type = LIGHT_TYPE::SPOT_LIGHT;
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
            ImGui::AcceptDragDropPayload("IMPORT_MODEL_ASSETS")) {
      char *modelPath = (char *)payload->Data;
      // Console.Log("[info]: import model from %s\n", modelPath);
      auto modelMeshes = Loader.GetModel(modelPath);
      auto parentEntity = GWORLD.AddNewEntity();
      parentEntity->name = fs::path(modelPath).stem().string();
      for (auto cmesh : modelMeshes) {
        auto childEntity = GWORLD.AddNewEntity();
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
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("IMPORT_MOTION")) {
      char *path = (char *)payload->Data;
      if (fs::path(path).extension().string() == ".bvh") {
        // handle bvh mocap import
        auto motion = Loader.GetMotion(path);
        auto parent = GWORLD.AddNewEntity();
        // attach animator component to a proxy entity
        parent->AddComponent<Animator>(motion);
        parent->name = fs::path(path).stem().string();
        auto root = GWORLD.AddNewEntity();
        root->name = motion->skeleton.jointNames[0];
        // build motion hierarchy
        CreateBVHSkeletonHierarchy(&motion->skeleton, 0, glm::vec3(0.0f), root);
        // set up variables for animator component
        parent->GetComponent<Animator>().skeleton = root;
        parent->GetComponent<Animator>().ShowSkeleton = true;
        // make skeleton hierarchy a child of proxy entity
        parent->AssignChild(root);
      }
    }
    ImGui::EndDragDropTarget();
  }
  ImGui::End();
}