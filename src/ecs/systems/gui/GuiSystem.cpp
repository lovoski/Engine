#include "global.hpp"
#include "engine/Engine.hpp"

#include "ecs/components/Material.hpp"
#include "ecs/components/Lights.hpp"
#include "ecs/components/Lights.hpp"
#include "ecs/components/Camera.hpp"
#include "ecs/components/MeshRenderer.hpp"

#include "ecs/systems/gui/GuiSystem.hpp"

void GuiSystem::Start() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGui::StyleColorsLight();

  // setup imguizmo
  mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
  mCurrentGizmoMode = ImGuizmo::WORLD;

  io = &ImGui::GetIO();
  io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  io->Fonts->AddFontFromFileTTF("./default/fonts/DSM/DroidSansMono.ttf", 20);

  io->IniFilename = layoutFileName;

  ImGui_ImplGlfw_InitForOpenGL(&Core.Window(), true);
  ImGui_ImplOpenGL3_Init("#version 460");
}

void GuiSystem::Destroy() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void GuiSystem::Reset() {
  // don't select anything
  selectedEntity = (ECS::EntityID)(-1);
}

void DrawProfiler() {}

void GuiSystem::MainMenuBar() {
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
        std::ofstream sceneFileOutput(Core.ActiveSceneFile);
        if (!sceneFileOutput.is_open()) {
          Console.Log("[error]: can't save scene to %s\n", Core.ActiveSceneFile.c_str());
        } else {
          sceneFileOutput << Core.DumpSceneAsJson();
          Console.Log("[info]: save scene to %s\n", Core.ActiveSceneFile.c_str());
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
        // ImGui::Separator();
        // ImGui::MenuItem("Grid Options", nullptr, nullptr, false);
        // ImGui::Checkbox("Show Grid", &showGizmoGrid);
        // ImGui::PushItemWidth(100);
        // ImGui::InputInt("Grid Size", &gizmoGridSize);
        // ImGui::PopItemWidth();
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

void GuiSystem::ConsoleWindow() { Console.Draw("Console"); }

void GuiSystem::DrawGizmos(float x, float y, float width, float height,
                               bool enable) {

  if (ImGui::IsKeyPressed(ImGuiKey_G))
    mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
  if (ImGui::IsKeyPressed(ImGuiKey_R))
    mCurrentGizmoOperation = ImGuizmo::ROTATE;
  if (ImGui::IsKeyPressed(ImGuiKey_S))
    mCurrentGizmoOperation = ImGuizmo::SCALE;

  ECS::EntityID camera;
  if (enable && Core.GetActiveCamera(camera)) {
    ImGuizmo::AllowAxisFlip(false);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(x, y, width, height);
    Entity *cameraEnt = Core.EManager.EntityFromID(camera);
    Camera &cameraComp = cameraEnt->GetComponent<Camera>();
    // if (showGizmoGrid) {
    //   ImGuizmo::DrawGrid(
    //       glm::value_ptr(cameraComp.GetViewMatrix(*cameraEnt)),
    //       glm::value_ptr(cameraComp.GetProjMatrixPerspective(width, height)),
    //       glm::value_ptr(mat4(1.0f)), gizmoGridSize);
    // }
    if (selectedEntity != (ECS::EntityID)(-1)) {
      Entity *selected = Core.EManager.EntityFromID(selectedEntity);
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

void GuiSystem::Render() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
  ImGui::Begin("Scene");
  ImGui::BeginChild("GameRenderer");
  auto size = ImGui::GetContentRegionAvail();
  auto pos = ImGui::GetWindowPos();
  ImGui::Image((void *)Core.SceneBuffer->GetFrameTexture(), size, ImVec2(0, 1),
               ImVec2(1, 0));
  if (Core.SceneWindowSize.x != size.x || Core.SceneWindowSize.y != size.y) {
    Core.SceneBuffer->RescaleFrameBuffer(size.x, size.y);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      Console.Log("[error]: Rescaled framebuffer is not complete\n");
    glViewport(0, 0, size.x, size.y);
  }
  Core.SceneWindowSize = {size.x, size.y};
  Core.SceneWindowPos = {pos.x, pos.y};
  DrawGizmos(pos.x, pos.y, size.x, size.y);
  ImGui::EndChild();
  ImGui::End();

  MainMenuBar();
  EntitiesWindow();
  ConsoleWindow();
  InspectorWindow();
  AssetsWindow();
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}