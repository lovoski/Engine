#include "Editor.hpp"

#include "Component/NativeScript.hpp"
#include "Scripts/CameraController.hpp"
#include "Scripts/TestDebugDraw.hpp"

void BuildTestScene(Engine *engine) {
  auto ent = GWORLD.AddNewEntity();
  ent->name = "Script Base";
  ent->AddComponent<aEngine::NativeScript>();
  ent->GetComponent<aEngine::NativeScript>().Bind<CameraController>();

  ent->AddComponent<aEngine::NativeScript>();
  ent->GetComponent<aEngine::NativeScript>().Bind<TestDebugDraw>();

  auto cam = GWORLD.AddNewEntity();
  cam->name = "Editor Cam";
  cam->AddComponent<Camera>();
  auto &camera = cam->GetComponent<Camera>();
  camera.zFar = 1000.0f;
  cam->SetGlobalPosition(glm::vec3(0.0f, 0.0f, 0.0f));
  GWORLD.SetActiveCamera(cam->ID);
}

Editor::Editor(int width, int height) {
  engine = new Engine(width, height);
  context.frameBuffer = new FrameBuffer(width, height);
  // setup editor context
  context.Reset();
}

void Editor::Start() {
  engine->Start();
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGui::StyleColorsLight();

  // setup imguizmo
  context.mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
  context.mCurrentGizmoMode = ImGuizmo::WORLD;

  context.io = &ImGui::GetIO();
  context.io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  context.io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  context.io->Fonts->AddFontFromFileTTF("./Assets/fonts/DSM/DroidSansMono.ttf",
                                        20);

  context.io->IniFilename = context.layoutFileName;

  ImGui_ImplGlfw_InitForOpenGL(GWORLD.Context.window, true);
  ImGui_ImplOpenGL3_Init("#version 460");

  // TODO: remove this in final version
  BuildTestScene(engine);
}

void Editor::Run(bool release) {
  if (release) {

    // this callback only works at release build
    engine->ResizeCallbacks.push_back([](Engine *engine, int w, int h) {
      glViewport(0, 0, w, h);
      context.frameBuffer->RescaleFrameBuffer(w, h);
      GWORLD.Context.sceneWindowSize = glm::vec2(w, h);
      GWORLD.RenderBegin();
      GWORLD.RenderEnd();
    });

    while (engine->Run()) {
      // logic update
      engine->Update();

      // directly render to default framebuffer
      engine->RenderBegin();
      engine->RenderEnd();
    }
  } else {
    // display the editor gui as usual
    while (engine->Run()) {
      engine->Update(); // logic update

      // start editor ui
      ImGui_ImplOpenGL3_NewFrame();
      ImGui_ImplGlfw_NewFrame();
      ImGui::NewFrame();

      ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
      ImGui::Begin("Scene");
      ImGui::BeginChild("GameRenderer");
      auto size = ImGui::GetContentRegionAvail();
      auto pos = ImGui::GetWindowPos();
      ImGui::Image((void *)context.frameBuffer->GetFrameTexture(), size,
                   ImVec2(0, 1), ImVec2(1, 0));
      if (GWORLD.Context.sceneWindowSize.x != size.x ||
          GWORLD.Context.sceneWindowSize.y != size.y) {
        context.frameBuffer->RescaleFrameBuffer(size.x, size.y);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
          Console.Log("[error]: Rescaled framebuffer is not complete\n");
        GWORLD.Context.sceneWindowSize = {size.x, size.y};
        glViewport(0, 0, size.x, size.y);
        // render additional frame to avoid flashing
        context.frameBuffer->Bind();
        engine->RenderBegin();
        context.frameBuffer->Unbind();
        engine->RenderEnd();
      }
      GWORLD.Context.sceneWindowPos = {pos.x, pos.y};
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

      // the engine will render scene to its framebuffer
      context.frameBuffer->Bind();
      engine->RenderBegin();
      context.frameBuffer->Unbind();
      engine->RenderEnd();
    }
  }
}

void Editor::Shutdown() {
  if (context.frameBuffer)
    delete context.frameBuffer;
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  engine->Shutdown();
}

void Editor::MainMenuBar() {
  static bool showProfiler = false;

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      ImGui::MenuItem("Project", nullptr, nullptr, false);
      if (ImGui::MenuItem("Open Folder")) {
        // switch activeBaseFolder
        IGFD::FileDialogConfig config;
        config.path = ".";
        ImGuiFileDialog::Instance()->OpenDialog(
            "chooseprojectrootdir", "Choose Project Root", nullptr, config);
      }
      if (ImGui::MenuItem("Save Scene", "CTRL+S")) {
        std::string sceneFilePath = GWORLD.Context.sceneFilePath;
        std::ofstream sceneFileOutput(sceneFilePath);
        if (!sceneFileOutput.is_open()) {
          Console.Log("[error]: can't save scene to %s\n",
                      sceneFilePath.c_str());
        } else {
          sceneFileOutput << GWORLD.Serialize();
          Console.Log("[info]: save scene to %s\n", sceneFilePath.c_str());
        }
        sceneFileOutput.close();
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Window")) {
      if (ImGui::BeginMenu("Gizmos")) {
        ImGui::MenuItem("Axis Space", nullptr, nullptr, false);
        if (ImGui::RadioButton("Local",
                               context.mCurrentGizmoMode == ImGuizmo::LOCAL))
          context.mCurrentGizmoMode = ImGuizmo::LOCAL;
        ImGui::SameLine();
        if (ImGui::RadioButton("World",
                               context.mCurrentGizmoMode == ImGuizmo::WORLD))
          context.mCurrentGizmoMode = ImGuizmo::WORLD;
        ImGui::Separator();
        ImGui::MenuItem("Grid Options", nullptr, nullptr, false);
        ImGui::Checkbox("Show Grid", &GWORLD.Context.showGrid);
        ImGui::PushItemWidth(100);
        int gridSize = GWORLD.Context.gridSize;
        ImGui::InputInt("Grid Size", &gridSize);
        GWORLD.Context.gridSize = gridSize;
        ImGui::PopItemWidth();
        ImGui::EndMenu();
      }
      if (ImGui::MenuItem("Show Profiler")) {
        showProfiler = true;
      }
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
      std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
      // Console.Log("filepathname:%s\nfilepath%s\n", filePathName.c_str(),
      // filePath.c_str());
      context.activeBaseFolder = filePath;
    }
    ImGuiFileDialog::Instance()->Close();
  }
  if (showProfiler) {
    ImGui::Begin("Profiler", &showProfiler);
    ImGui::MenuItem("Frames Per Second:", nullptr, nullptr, false);
    ImGui::Text("%d", (int)(1.0 / GWORLD.Context.deltaTime));
    ImGui::MenuItem("Hierarchy Update:", nullptr, nullptr, false);
    ImGui::Text("%.4f ms", GWORLD.Context.hierarchyUpdateTime * 1000);
    ImGui::MenuItem("Main Update:", nullptr, nullptr, false);
    ImGui::Text("%.4f ms", GWORLD.Context.updateTime * 1000);
    ImGui::MenuItem("Main Render:", nullptr, nullptr, false);
    ImGui::Text("%.4f ms", GWORLD.Context.renderTime * 1000);
    ImGui::MenuItem("Debug Render:", nullptr, nullptr, false);
    ImGui::Text("%.4f ms", GWORLD.Context.debugDrawTime * 1000);
    ImGui::End();
  }
}

void Editor::ConsoleWindow() { Console.Draw("Console"); }

void Editor::DrawGizmos(float x, float y, float width, float height,
                        bool enable) {

  auto pos = GWORLD.Context.currentMousePosition;
  // only change the gizmo operation mode
  // if the cursor is inside scene window
  if (GWORLD.InSceneWindow(pos.x, pos.y)) {
    if (ImGui::IsKeyPressed(ImGuiKey_G))
      context.mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R))
      context.mCurrentGizmoOperation = ImGuizmo::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_S))
      context.mCurrentGizmoOperation = ImGuizmo::SCALE;
  }

  EntityID camera;
  if (enable && GWORLD.GetActiveCamera(camera)) {
    ImGuizmo::AllowAxisFlip(false);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(x, y, width, height);
    Entity *cameraEnt = GWORLD.EntityFromID(camera);
    Camera &cameraComp = cameraEnt->GetComponent<Camera>();
    if (context.selectedEntity != (EntityID)(-1)) {
      Entity *selected = GWORLD.EntityFromID(context.selectedEntity);
      glm::mat4 modelTransform = selected->GetModelMatrix();
      if (ImGuizmo::Manipulate(
          glm::value_ptr(cameraComp.GetViewMatrix(*cameraEnt)),
          glm::value_ptr(cameraComp.GetProjMatrixPerspective(width, height)),
          context.mCurrentGizmoOperation, context.mCurrentGizmoMode,
          glm::value_ptr(modelTransform), NULL, NULL)) {
        // update object transform with modified changes
        if (context.mCurrentGizmoOperation == ImGuizmo::TRANSLATE) {
          glm::vec3 position(modelTransform[3][0], modelTransform[3][1],
                             modelTransform[3][2]);
          selected->SetGlobalPosition(position);
        } else {
          glm::vec3 scale(glm::length(modelTransform[0]),
                          glm::length(modelTransform[1]),
                          glm::length(modelTransform[2]));
          if (context.mCurrentGizmoOperation == ImGuizmo::ROTATE) {
            glm::mat4 rotation = glm::mat4(
                modelTransform[0] / scale.x, modelTransform[1] / scale.y,
                modelTransform[2] / scale.z, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
            selected->SetGlobalRotation(glm::quat_cast(rotation));
          } else if (context.mCurrentGizmoOperation == ImGuizmo::SCALE)
            selected->SetGlobalScale(scale);
        }
      }
    }
  }
}
