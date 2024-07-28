#include "EditorWindows.hpp"
#include "ecs/systems/hierarchy/HierarchySystem.hpp"
#include "roboto.h"

void EditorWindows::Initialize() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  io = &ImGui::GetIO();
  io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  io->Fonts->AddFontFromMemoryTTF(Roboto_Regular_ttf, Roboto_Regular_ttf_len,
                                  20.0f);

  io->IniFilename = layoutFileName;

  ImGui_ImplGlfw_InitForOpenGL(&Core.Window(), true);
  ImGui_ImplOpenGL3_Init("#version 460");
}

void EditorWindows::Destroy() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void EditorWindows::MainMenuBar() {}

inline void DrawHierarchyGUI(ECS::Entity *entity, ECS::EntityID &selectedEntity,
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
                              sizeof(ECS::Entity *));
    ImGui::Text("Drag drop to change hierarchy");
    ImGui::EndDragDropSource();
  }
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("CHANGE_ENTITY_HIERARCHY")) {
      ECS::Entity *newChild = *(ECS::Entity **)payload->Data;
      ECS::EManager.GetSystemInstance<HierarchySystem>()
          ->AttachNewChildToParent(newChild, entity);
    }
    ImGui::EndDragDropTarget();
  }
  // right click context menu
  if (ImGui::BeginPopupContextItem((entity->name + " popup").c_str(),
                                   ImGuiPopupFlags_MouseButtonRight)) {
    ImGui::SeparatorText("Entity Options");
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
  ImGui::BeginChild("Entities List", {-1, ImGui::GetContentRegionAvail().y});
  auto entities =
      ECS::EManager.GetSystemInstance<HierarchySystem>()->hierarchyEntityRoots;
  static ImGuiTreeNodeFlags guiTreeNodeFlags =
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
      ImGuiTreeNodeFlags_SpanAvailWidth;
  for (auto i = 0; i < entities.size(); ++i) {
    DrawHierarchyGUI(entities[i], selectedEntity, guiTreeNodeFlags);
  }
  ImGui::EndChild();
  ImGui::End();
}

void EditorWindows::ConsoleWindow() { Console.Draw("Console"); }

void EditorWindows::AssetsWindow() {
  ImGui::Begin("Assets");
  ImGui::End();
}

inline void DrawTransformGUI(ECS::EntityID selectedEntity) {
  auto &transform = ECS::EManager.GetComponent<Transform>(selectedEntity);
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

    auto &euler = transform.EulerAngles;
    ImGui::SeparatorText("Rotation");
    bool updateRotation = false;
    updateRotation |=
        ImGui::DragFloat(" :rot.X", &euler.x, 0.001f, -MAX_FLOAT, MAX_FLOAT);
    updateRotation |=
        ImGui::DragFloat(" :rot.Y", &euler.y, 0.001f, -MAX_FLOAT, MAX_FLOAT);
    updateRotation |=
        ImGui::DragFloat(" :rot.Z", &euler.z, 0.001f, -MAX_FLOAT, MAX_FLOAT);
    ImGui::TreePop();
  }
}

inline void DrawCameraGUI(ECS::EntityID selectedEntity) {
  auto &camera = ECS::EManager.GetComponent<Camera>(selectedEntity);
  if (ImGui::TreeNode("Camera")) {
    ImGui::DragFloat(" :Fov  Y", &camera.fovY, 1.0f, 0.0f, 150.0f);
    ImGui::DragFloat(" :Z Near", &camera.zNear, 0.001f, 0.0000001f, 10.0f);
    ImGui::DragFloat(" :Z  Far", &camera.zFar, 0.1f, 20.0f, 2000.0f);
    ImGui::TreePop();
  }
}

inline void DrawBaseMaterialGUI(ECS::EntityID selectedEntity) {
  auto &material = ECS::EManager.GetComponent<BaseMaterial>(selectedEntity);
  // Console.Log("vertex shader: %s\nfragment shader: %s\n",
  //             material.VertShader.c_str(), material.FragShader.c_str());
  if (ImGui::TreeNode("Base Material")) {
    ImGui::Separator();
    // ImGui::Checkbox("Pass to children", &material.forceChildSameMaterial);

    ImGui::Separator();
    ImGui::ColorEdit3("Albedo", material.Albedo);

    ImGui::Separator();
    ImGui::SliderFloat("Ambient", &material.Ambient, 0.0f, 1.0f);

    ImGui::TreePop();
  }
}

inline void DrawBaseLightGUI(ECS::EntityID selectedEntity) {
  auto &light = ECS::EManager.GetComponent<BaseLight>(selectedEntity);
  if (ImGui::TreeNode("Base Light")) {
    const char *comboItems[] = {"Directional light", "Point light",
                                "Spot light"};
    static int baseLightGUIComboItemIndex = 0;
    ImGui::Combo("Light Type", &baseLightGUIComboItemIndex, comboItems, 3);
    if (baseLightGUIComboItemIndex == 0) {
      ImGui::ColorEdit3("Light Color", light.LightColor);
    } else if (baseLightGUIComboItemIndex == 1) {
    } else if (baseLightGUIComboItemIndex == 2) {
    }
    ImGui::TreePop();
  }
}

void EditorWindows::ComponentsWindow() {
  ImGui::Begin("Components");
  if (ImGui::Button("Add Component", {-1, 40})) {
    Console.Log("add component\n");
  }
  if (selectedEntity != (ECS::EntityID)(-1)) {
    string entityName =
        "Active Entity : " + ECS::EManager.EntityFromID(selectedEntity)->name;
    ImGui::SeparatorText(entityName.c_str());
    ImGui::BeginChild("Components List",
                      {-1, ImGui::GetContentRegionAvail().y});
    if (ECS::EManager.HasComponent<Transform>(selectedEntity))
      DrawTransformGUI(selectedEntity);
    if (ECS::EManager.HasComponent<Camera>(selectedEntity))
      DrawCameraGUI(selectedEntity);
    if (ECS::EManager.HasComponent<BaseMaterial>(selectedEntity))
      DrawBaseMaterialGUI(selectedEntity);
    if (ECS::EManager.HasComponent<BaseLight>(selectedEntity))
      DrawBaseLightGUI(selectedEntity);
    ImGui::EndChild();
  }
  ImGui::End();
}

void EditorWindows::RenderStart(Graphics::FrameBuffer *sceneBuffer) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  MainMenuBar();

  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
  ImGui::Begin("Scene");
  ImGui::BeginChild("GameRenderer");
  auto size = ImGui::GetContentRegionAvail();
  auto pos = ImGui::GetWindowPos();
  ImGui::Image((void *)sceneBuffer->GetFrameTexture(), size, ImVec2(1, 1),
               ImVec2(0, 0));
  SceneWindowSize = {size.x, size.y};
  SceneWindowPos = {pos.x, pos.y};
  ImGui::EndChild();
  ImGui::End();

  ImGui::ShowDemoWindow();

  EntitiesWindow();
  ConsoleWindow();
  ComponentsWindow();
  AssetsWindow();
}

void EditorWindows::RenderComplete() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool EditorWindows::LoopCursorInSceneWindow() {
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

bool EditorWindows::SetActiveCamera(ECS::EntityID camera) {
  if (ECS::EManager.HasComponent<Camera>(camera) &&
      ECS::EManager.HasComponent<Transform>(camera)) {
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

bool EditorWindows::GetActiveCamera(ECS::EntityID &camera) {
  if (hasActiveCamera) {
    camera = activeCamera;
    return true;
  } else {
    Console.Log("[Info]: There's no active camera\n");
    camera = (ECS::EntityID)(-1);
    return false;
  }
}