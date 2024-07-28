#include "EditorWindows.hpp"

void EditorWindows::Initialize() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  io = &ImGui::GetIO();
  io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  ImFontConfig fontConfig;
  fontConfig.SizePixels = 20.0f;
  io->Fonts->AddFontDefault(&fontConfig);
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

void EditorWindows::EntitiesWindow() {
  ImGui::Begin("Entities");
  if (ImGui::Button("Add Entity", ImVec2(-1, 40))) {
    Console.Log("add entity\n");
  }
  ImGui::SeparatorText("Scene");
  ImGui::BeginChild("Entities List", {-1, ImGui::GetContentRegionAvail().y});
  auto entities = ECS::EManager.GetActiveEntities();
  for (auto i = 0; i < entities.size(); ++i) {
    if (ImGui::Selectable(entities[i]->name.c_str(), selectedEntityInd == i)) {
      selectedEntityInd = i;
      selectedEntity = entities[i]->ID;
    }
    // TODO: refine this menu
    if (ImGui::BeginPopupContextItem((entities[i]->name + " popup").c_str(),
                                     ImGuiPopupFlags_MouseButtonRight)) {
      if (ImGui::Button("remove")) {
        Console.Log("remove entity\n");
        ECS::EManager.DestroyEntity(entities[i]->ID);
        selectedEntity = (ECS::EntityID)(-1);
        selectedEntityInd = -1;
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }
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