#include "EditorWindows.hpp"
#include "resource/MaterialData.hpp"
#include "roboto.h"

void EditorWindows::Initialize() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGui::StyleColorsLight();

  // setup imguizmo
  mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
  mCurrentGizmoMode = ImGuizmo::WORLD;

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

void EditorWindows::Reset() {
  // entity states
  // reset camera status
  activeCamera = (ECS::EntityID)(-1);
  hasActiveCamera = false;
  selectedEntity = (ECS::EntityID)(-1);

  // gizmos state
  mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
  mCurrentGizmoMode = ImGuizmo::WORLD;
}

void DrawProfiler() {}

void EditorWindows::MainMenuBar() {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      ImGui::MenuItem("Project", nullptr, nullptr, false);
      if (ImGui::MenuItem("Open Folder")) {
        // switch activeBaseFolder
        IGFD::FileDialogConfig config;
        config.path = ".";
        ImGuiFileDialog::Instance()->OpenDialog("chooseprojectrootdir", "Choose Project Root", nullptr, config);
      }
      if (ImGui::MenuItem("Save Scene", "CTRL+S")) {
        std::ofstream sceneFileOutput(activeSceneFile);
        if (!sceneFileOutput.is_open()) {
          Console.Log("[error]: can't save scene to %s\n", activeSceneFile.c_str());
        } else {
          sceneFileOutput << ECS::EManager.CaptureStatesAsScene();
          Console.Log("[info]: save scene to %s\n", activeSceneFile.c_str());
        }
        sceneFileOutput.close();
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Window")) {
      if (ImGui::BeginMenu("Gizmos")) {
        ImGui::MenuItem("Axis Space", nullptr, nullptr, false);
        if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
          mCurrentGizmoMode = ImGuizmo::LOCAL;
        ImGui::SameLine();
        if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
          mCurrentGizmoMode = ImGuizmo::WORLD;
        ImGui::Separator();
        ImGui::MenuItem("Grid Options", nullptr, nullptr, false);
        ImGui::Checkbox("Show Grid", &showGizmoGrid);
        ImGui::PushItemWidth(100);
        ImGui::InputInt("Grid Size", &gizmoGridSize);
        ImGui::PopItemWidth();
        ImGui::EndMenu();
      }
      // if (ImGui::MenuItem("Show Profiler")) {
      //   DrawProfiler();
      // }
      // if (ImGui::MenuItem("Show Style Editor")) {
      //   ImGui::ShowStyleEditor();
      // }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
  if (ImGuiFileDialog::Instance()->Display("chooseprojectrootdir")) {
    if (ImGuiFileDialog::Instance()->IsOk()) {
      // string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
      string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
      // Console.Log("filepathname:%s\nfilepath%s\n", filePathName.c_str(), filePath.c_str());
      activeBaseFolder = filePath;
    }
    ImGuiFileDialog::Instance()->Close();
  }
}

void EditorWindows::ConsoleWindow() { Console.Draw("Console"); }

void EditorWindows::DrawGizmos(float x, float y, float width, float height,
                               bool enable) {

  if (ImGui::IsKeyPressed(ImGuiKey_G))
    mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
  if (ImGui::IsKeyPressed(ImGuiKey_R))
    mCurrentGizmoOperation = ImGuizmo::ROTATE;
  if (ImGui::IsKeyPressed(ImGuiKey_S))
    mCurrentGizmoOperation = ImGuizmo::SCALE;

  ECS::EntityID camera;
  if (enable && GetActiveCamera(camera)) {
    ImGuizmo::AllowAxisFlip(false);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(x, y, width, height);
    Entity *cameraEnt = ECS::EManager.EntityFromID(camera);
    Camera &cameraComp = cameraEnt->GetComponent<Camera>();
    if (showGizmoGrid) {
      ImGuizmo::DrawGrid(
          glm::value_ptr(cameraComp.GetViewMatrix(*cameraEnt)),
          glm::value_ptr(cameraComp.GetProjMatrixPerspective(width, height)),
          glm::value_ptr(mat4(1.0f)), gizmoGridSize);
    }
    if (selectedEntity != (ECS::EntityID)(-1)) {
      Entity *selected = ECS::EManager.EntityFromID(selectedEntity);
      mat4 modelTransform = selected->GetModelMatrix();
      ImGuizmo::Manipulate(
          glm::value_ptr(cameraComp.GetViewMatrix(*cameraEnt)),
          glm::value_ptr(cameraComp.GetProjMatrixPerspective(width, height)),
          mCurrentGizmoOperation, mCurrentGizmoMode,
          glm::value_ptr(modelTransform), NULL, NULL);
      if (ImGuizmo::IsUsing()) {
        // update object transform with modified changes
        if (mCurrentGizmoOperation == ImGuizmo::TRANSLATE) {
          vec3 position(modelTransform[3][0], modelTransform[3][1],
                        modelTransform[3][2]);
          selected->SetGlobalPosition(position);
        } else {
          vec3 scale(glm::length(modelTransform[0]),
                     glm::length(modelTransform[1]),
                     glm::length(modelTransform[2]));

          if (mCurrentGizmoOperation == ImGuizmo::ROTATE) {
            mat4 rotation =
                mat4(modelTransform[0] / scale.x, modelTransform[1] / scale.y,
                     modelTransform[2] / scale.z, vec4(0.0f, 0.0f, 0.0f, 1.0f));
            selected->SetGlobalRotation(glm::quat_cast(rotation));
          } else if (mCurrentGizmoOperation == ImGuizmo::SCALE)
            selected->SetGlobalScale(scale);
        }
      }
    }
  }
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
  ImGui::Image((void *)sceneBuffer->GetFrameTexture(), size, ImVec2(0, 1),
               ImVec2(1, 0));
  SceneWindowSize = {size.x, size.y};
  SceneWindowPos = {pos.x, pos.y};
  DrawGizmos(pos.x, pos.y, size.x, size.y);
  ImGui::EndChild();
  ImGui::End();

  EntitiesWindow();
  ConsoleWindow();
  InspectorWindow();
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
  if (camera == (ECS::EntityID)(-1)) {
    Console.Log("[info]: camera is not a valid entity\n");
    return false;
  }
  if (ECS::EManager.HasComponent<Camera>(camera)) {
    hasActiveCamera = true;
    activeCamera = camera;
    Console.Log("[info]: set active camera to %s\n", ECS::EManager.EntityFromID(camera)->name.c_str());
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
    // Console.Log("[Info]: There's no active camera\n");
    camera = (ECS::EntityID)(-1);
    return false;
  }
}