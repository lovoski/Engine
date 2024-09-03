#include "Editor.hpp"

#include <tinyfiledialogs.h>

#include "Component/MeshRenderer.hpp"
#include "Component/NativeScript.hpp"
#include "Scripts/CameraController.hpp"
#include "Scripts/TestDebugDraw.hpp"
#include "System/Animation/AnimationSystem.hpp"
#include "System/Render/CameraSystem.hpp"
#include "System/Render/RenderSystem.hpp"

void BuildTestScene(Engine *engine) {
  GWORLD.SetupDefaultScene();

  auto sphere = GWORLD.AddNewEntity();
  sphere->name = "Sphere";
  sphere->SetGlobalPosition({0.0f, 1.0f, 0.0f});
  sphere->AddComponent<MeshRenderer>(Loader.GetMesh("::spherePrimitive", ""));
  sphere->GetComponent<MeshRenderer>()->AddPass<Render::OutlinePass>(
      nullptr, "Outline Pass");
  sphere->GetComponent<MeshRenderer>()->AddPass<Render::Diffuse>(
      nullptr, "Diffuse Sphere");

  auto plane = GWORLD.AddNewEntity();
  plane->name = "Ground";
  plane->SetGlobalScale({10.0f, 1.0f, 10.0f});
  plane->AddComponent<MeshRenderer>(Loader.GetMesh("::planePrimitive", ""));
  plane->GetComponent<MeshRenderer>()->AddPass<Render::Diffuse>(nullptr,
                                                                "Ground Mat");

  // auto cube = GWORLD.AddNewEntity();
  // cube->name = "Cube";
  // cube->AddComponent<MeshRenderer>(Loader.GetMesh("::cubePrimitive", ""));
  // cube->GetComponent<MeshRenderer>().AddPass<Render::Diffuse>(
  //     nullptr, "Cube Mat");
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

  ImGui::StyleColorsDark();
  auto &style = ImGui::GetStyle();
  style.FrameBorderSize = 1.0f;
  style.FrameRounding = 3.0f;

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
      GWORLD.ForceRender();
      GWORLD.RenderEnd();
    });

    while (engine->Run()) {
      // directly render to default framebuffer
      engine->Update();
      engine->RenderEnd();
    }
  } else {
    // display the editor gui as usual
    while (engine->Run()) {
      // the engine will render scene to its framebuffer
      context.frameBuffer->Bind();
      engine->Update(); // logic update
      context.frameBuffer->Unbind();

      float currentTime = GWORLD.GetTime();
      context.editorDeltaRender = currentTime - context.editorLastRender;
      context.editorLastRender = currentTime;

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
          LOG_F(INFO, "Rescaled framebuffer is not complete");
        GWORLD.Context.sceneWindowSize = {size.x, size.y};
        glViewport(0, 0, size.x, size.y);
        // render additional frame to avoid flashing
        context.frameBuffer->Bind();
        GWORLD.ForceRender();
        context.frameBuffer->Unbind();
        engine->RenderEnd();
      }
      GWORLD.Context.sceneWindowPos = {pos.x, pos.y};
      DrawGizmos(pos.x, pos.y, size.x, size.y);
      ImGui::EndChild();
      ImGui::End();

      MainMenuBar();
      if (showMainSequencer)
        MainSequencer();
      if (showEntitiesWindow)
        EntitiesWindow();
      if (showInspectorWindow)
        InspectorWindow();
      if (showAssetsWindow)
        AssetsWindow();
      ImGui::EndFrame();
      ImGui::Render();
      ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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

void Editor::MainSequencer() {
  auto animSystem = GWORLD.GetSystemInstance<AnimationSystem>();
  ImGui::Begin("Timeline", &showMainSequencer);
  ImGui::BeginChild("TimelineProperties", {-1, 50});
  ImGui::Checkbox("Auto Play", &animSystem->EnableAutoPlay);
  ImGui::SameLine();
  int se[2] = {animSystem->SystemStartFrame, animSystem->SystemEndFrame};
  ImGui::PushItemWidth(-1);
  if (ImGui::InputInt2("##Start & End", se)) {
    animSystem->SystemStartFrame = se[0];
    animSystem->SystemEndFrame = se[1];
  }
  ImGui::PopItemWidth();
  ImGui::EndChild();
  ImGui::BeginChild("TimelineView", {-1, 60});
  ImGui::PushItemWidth(-1);
  ImGui::SliderFloat("##currenrFrame", &animSystem->SystemCurrentFrame,
                     animSystem->SystemStartFrame, animSystem->SystemEndFrame);
  ImGui::PopItemWidth();
  ImGui::EndChild();
  ImGui::End();
}

void Editor::MainMenuBar() {
  static bool showProfiler = false;

  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      ImGui::MenuItem("Project", nullptr, nullptr, false);
      ImGui::Separator();
      if (ImGui::MenuItem("Open Folder")) {
        auto result = tinyfd_selectFolderDialog(
            "Select Base Folder", context.activeBaseFolder.c_str());
        if (result != NULL) {
          context.activeBaseFolder = result;
        }
      }
      ImGui::MenuItem("Scene", nullptr, nullptr, false);
      ImGui::Separator();
      if (ImGui::MenuItem("Clear Scene")) {
        GWORLD.Reset();
        GWORLD.SetupDefaultScene();
      }
      if (ImGui::MenuItem("Save Scene", "CTRL+S")) {
        std::string sceneFilePath = GWORLD.Context.sceneFilePath;
        if (sceneFilePath == "::defaultScene") {
          const char *filters[] = {"*.scene"};
          auto result = tinyfd_saveFileDialog(
              "Save Scene", (context.activeBaseFolder + "/").c_str(), 1,
              filters, "Scene File");
          if (result != NULL) {
            LOG_F(INFO, "save default scene to %s", result);
          }
        } else {
          std::ofstream sceneFileOutput(sceneFilePath);
          if (!sceneFileOutput.is_open()) {
            LOG_F(ERROR, "can't save scene to %s", sceneFilePath.c_str());
          } else {
            // sceneFileOutput << GWORLD.Serialize();
            LOG_F(INFO, "save scene to %s", sceneFilePath.c_str());
          }
          sceneFileOutput.close();
        }
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Systems")) {
      if (ImGui::BeginMenu("Render")) {
        auto renderSystem = GWORLD.GetSystemInstance<RenderSystem>();
        auto cameraSystem = GWORLD.GetSystemInstance<CameraSystem>();
        ImGui::PushItemWidth(180);
        ImGui::MenuItem("Shadows", nullptr, nullptr, false);
        ImGui::Checkbox("Enable Shadow", &renderSystem->EnableShadowMaps);
        std::vector<const char *> shadowMapSizeSlectable{
            "64", "128", "256", "512", "1024", "2048", "4096"};
        static int currentShadowMapSizeSlectableIndex = 4;
        if (ImGui::Combo("Shadow Map Size", &currentShadowMapSizeSlectableIndex,
                         shadowMapSizeSlectable.data(),
                         shadowMapSizeSlectable.size())) {
          renderSystem->GlobalShadowMapSize = std::stoi(
              shadowMapSizeSlectable[currentShadowMapSizeSlectableIndex]);
          renderSystem->ResizeAllShadowMaps();
        }
        ImGui::Separator();
        ImGui::MenuItem("Camera", nullptr, nullptr, false);
        std::vector<std::string> cameraNames;
        std::map<EntityID, std::size_t> cameraToComboMap;
        cameraNames.push_back("None:-1");
        cameraToComboMap.insert(std::make_pair((EntityID)(-1), 0));
        auto availableCamera = cameraSystem->GetAvailableCamera();
        for (auto availCam : availableCamera) {
          auto cameraToComboMapSize = cameraToComboMap.size();
          cameraToComboMap.insert(
              std::make_pair(availCam->ID, cameraToComboMapSize));
          auto cameraName = availCam->name + ":" + std::to_string(availCam->ID);
          cameraNames.push_back(cameraName);
        }
        EntityID camera;
        static int currentActiveCameraComboIndex = 0;
        if (GWORLD.GetActiveCamera(camera)) {
          auto it = cameraToComboMap.find(camera);
          if (it == cameraToComboMap.end()) {
            LOG_F(ERROR, "Inconsistent camera combo and active camera");
          } else {
            currentActiveCameraComboIndex = it->second;
          }
        }
        if (ImGui::BeginCombo(
                "Active Camera",
                cameraNames[currentActiveCameraComboIndex].c_str())) {
          for (int comboIndex = 0; comboIndex < cameraNames.size();
               ++comboIndex) {
            bool isSelected = currentActiveCameraComboIndex == comboIndex;
            if (ImGui::Selectable(cameraNames[comboIndex].c_str(),
                                  isSelected)) {
              currentActiveCameraComboIndex = comboIndex;
              // switch active camera
              for (auto cameraComboMapPair : cameraToComboMap) {
                auto cameraEntityID = cameraComboMapPair.first;
                if (cameraComboMapPair.second ==
                    currentActiveCameraComboIndex) {
                  if (GWORLD.EntityValid(cameraEntityID)) {
                    GWORLD.SetActiveCamera(cameraEntityID);
                  } else {
                    LOG_F(ERROR, "Can't set active camera to a invalid entity");
                  }
                }
              }
            }
            if (isSelected)
              ImGui::SetItemDefaultFocus();
          }
          ImGui::EndCombo();
        }
        ImGui::PopItemWidth();
        ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Animation")) {
        auto animationSystem = GWORLD.GetSystemInstance<AnimationSystem>();
        ImGui::MenuItem("Default Variables", nullptr, nullptr, false);
        ImGui::Separator();
        ImGui::PushItemWidth(120);
        ImGui::InputInt("System FPS", &animationSystem->SystemFPS);
        ImGui::Checkbox("Auto Play", &animationSystem->EnableAutoPlay);
        ImGui::PopItemWidth();
        ImGui::EndMenu();
      }
      ImGui::EndMenu();
    }
    // if (ImGui::BeginMenu("Context")) {
    //   auto cameraSystem = GWORLD.GetSystemInstance<CameraSystem>();
    //   auto cameras = cameraSystem->GetAvailableCamera();
    //   std::vector<char *> names;
    //   ImGui::EndMenu();
    // }
    if (ImGui::BeginMenu("Window")) {
      if (ImGui::BeginMenu("Gizmos")) {
        auto renderSystem = GWORLD.GetSystemInstance<RenderSystem>();
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
        ImGui::Checkbox("Show Grid", &renderSystem->ShowGrid);
        float gridColor[3] = {renderSystem->GridColor.x,
                              renderSystem->GridColor.y,
                              renderSystem->GridColor.z};
        if (ImGui::ColorEdit3("##Grid Color", gridColor)) {
          renderSystem->GridColor =
              glm::vec3(gridColor[0], gridColor[1], gridColor[2]);
        }
        ImGui::PushItemWidth(120);
        int gridSize = renderSystem->GridSize;
        if (ImGui::InputInt("Grid Size", &gridSize))
          renderSystem->GridSize = gridSize;
        int gridSpacing = renderSystem->GridSpacing;
        if (ImGui::InputInt("Grid Spacing", &gridSpacing))
          renderSystem->GridSpacing = gridSpacing;
        ImGui::PopItemWidth();
        ImGui::EndMenu();
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Show Profiler"))
        showProfiler = true;
      if (ImGui::MenuItem("Show Sequencer"))
        showMainSequencer = true;
      ImGui::Separator();
      if (ImGui::MenuItem("Show Inspector"))
        showInspectorWindow = true;
      if (ImGui::MenuItem("Show Entities"))
        showEntitiesWindow = true;
      if (ImGui::MenuItem("Show Assets"))
        showAssetsWindow = true;
      // if (ImGui::MenuItem("Show Style Editor")) {
      //   ImGui::ShowStyleEditor();
      // }
      ImGui::EndMenu();
    }
    ImGui::EndMainMenuBar();
  }
  if (showProfiler) {
    ImGui::Begin("Profiler", &showProfiler);
    ImGui::MenuItem("Scene Profile", nullptr, nullptr, false);
    GWORLD.PlotSceneProfile();
    ImGui::End();
  }
}

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
    Entity *cameraEnt = GWORLD.EntityFromID(camera).get();
    auto cameraComp = cameraEnt->GetComponent<Camera>();
    if (GWORLD.EntityValid(context.selectedEntity)) {
      Entity *selected = GWORLD.EntityFromID(context.selectedEntity).get();
      glm::mat4 modelTransform = selected->GetModelMatrix();
      if (ImGuizmo::Manipulate(glm::value_ptr(cameraComp->ViewMat),
                               glm::value_ptr(cameraComp->ProjMat),
                               context.mCurrentGizmoOperation,
                               context.mCurrentGizmoMode,
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
