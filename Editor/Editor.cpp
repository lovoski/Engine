#include "Editor.hpp"

#include "Component/NativeScript.hpp"
#include "Scripts/CameraController.hpp"

void BuildTestScene(Engine *engine) {
  auto ent = GWORLD.AddNewEntity();
  ent->AddComponent<aEngine::NativeScript>();
  ent->GetComponent<aEngine::NativeScript>().Bind<CameraController>();

  auto cam = GWORLD.AddNewEntity();
  cam->AddComponent<Camera>();
  GWORLD.SetActiveCamera(cam->ID);
}

Editor::Editor(int width, int height) {
  engine = new Engine(width, height);
  quadShader = new Render::Shader();
  float quadVertices[] = {// positions   // texCoords
                          -1.0f, 1.0f,  0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
                          1.0f,  -1.0f, 1.0f, 0.0f, -1.0f, 1.0f,  0.0f, 1.0f,
                          1.0f,  -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  1.0f, 1.0f};
  glGenBuffers(1, &quadVBO);
  glGenVertexArrays(1, &quadVAO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices,
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));
  glBindVertexArray(0);
  quadShader->LoadAndRecompileShader("./Assets/shaders/quad.vert",
                                     "./Assets/shaders/quad.frag");

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

  context.io->Fonts->AddFontFromFileTTF("./Assets/fonts/DSM/DroidSansMono.ttf", 20);

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
      GWORLD.Context.sceneWindowSize = glm::vec2(w, h);
      GWORLD.Context.frameBuffer->RescaleFrameBuffer(w, h);
      GWORLD.RenderBegin();
      GWORLD.RenderEnd();
    });

    while (engine->Run()) {
      engine->Update(); // logic update
      engine->RenderBegin();
      quadShader->Use();
      glActiveTexture(GL_TEXTURE0 + 1);
      std::string name = "bufTex";
      int location = glGetUniformLocation(quadShader->ID, name.c_str());
      // if (location == -1) {
      //   printf("[warning]: uniform %s not found in shader\n", name.c_str());
      //   continue;
      // }
      glUniform1i(location, 1);
      glBindTexture(GL_TEXTURE_2D,
                    GWORLD.Context.frameBuffer->GetFrameTexture());
      glBindVertexArray(quadVAO);
      glDrawArrays(GL_TRIANGLES, 0, 6);
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
      ImGui::Image(
          (void*)GWORLD.Context.frameBuffer->GetFrameTexture(),
          size, ImVec2(0, 1), ImVec2(1, 0));
      if (GWORLD.Context.sceneWindowSize.x != size.x ||
          GWORLD.Context.sceneWindowSize.y != size.y) {
        GWORLD.Context.frameBuffer->RescaleFrameBuffer(size.x,
                                                                    size.y);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
          Console.Log("[error]: Rescaled framebuffer is not complete\n");
        GWORLD.Context.sceneWindowSize = {size.x, size.y};
        glViewport(0, 0, size.x, size.y);
        // render additional frame to avoid flashing
        engine->RenderBegin();
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
      engine->RenderBegin();
      engine->RenderEnd();
    }
  }
}

void Editor::Shutdown() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  engine->Shutdown();
}

void Editor::MainMenuBar() {
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
      std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
      // Console.Log("filepathname:%s\nfilepath%s\n", filePathName.c_str(),
      // filePath.c_str());
      context.activeBaseFolder = filePath;
    }
    ImGuiFileDialog::Instance()->Close();
  }
}

void Editor::ConsoleWindow() { Console.Draw("Console"); }

void Editor::DrawGizmos(float x, float y, float width, float height,
                        bool enable) {

  // auto pos = Event.MouseCurrentPosition;
  // // only change the gizmo operation mode
  // // if the cursor is inside scene window
  // if (Core.InSceneWindow(pos.x, pos.y)) {
  //   if (ImGui::IsKeyPressed(ImGuiKey_G))
  //     mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
  //   if (ImGui::IsKeyPressed(ImGuiKey_R))
  //     mCurrentGizmoOperation = ImGuizmo::ROTATE;
  //   if (ImGui::IsKeyPressed(ImGuiKey_S))
  //     mCurrentGizmoOperation = ImGuizmo::SCALE;
  // }

  // ECS::EntityID camera;
  // if (enable && Core.GetActiveCamera(camera)) {
  //   ImGuizmo::AllowAxisFlip(false);
  //   ImGuizmo::SetDrawlist();
  //   ImGuizmo::SetRect(x, y, width, height);
  //   Entity *cameraEnt = Core.EManager.EntityFromID(camera);
  //   Camera &cameraComp = cameraEnt->GetComponent<Camera>();
  //   if (selectedEntity != (ECS::EntityID)(-1)) {
  //     Entity *selected = Core.EManager.EntityFromID(selectedEntity);
  //     mat4 modelTransform = selected->GetModelMatrix();
  //     ImGuizmo::Manipulate(
  //         glm::value_ptr(cameraComp.GetViewMatrix(*cameraEnt)),
  //         glm::value_ptr(cameraComp.GetProjMatrixPerspective(width, height)),
  //         mCurrentGizmoOperation, mCurrentGizmoMode,
  //         glm::value_ptr(modelTransform), NULL, NULL);
  //     if (ImGuizmo::IsUsing()) {
  //       // update object transform with modified changes
  //       if (mCurrentGizmoOperation == ImGuizmo::TRANSLATE) {
  //         vec3 position(modelTransform[3][0], modelTransform[3][1],
  //                       modelTransform[3][2]);
  //         selected->SetGlobalPosition(position);
  //       } else {
  //         vec3 scale(glm::length(modelTransform[0]),
  //                    glm::length(modelTransform[1]),
  //                    glm::length(modelTransform[2]));

  //         if (mCurrentGizmoOperation == ImGuizmo::ROTATE) {
  //           mat4 rotation =
  //               mat4(modelTransform[0] / scale.x, modelTransform[1] /
  //               scale.y,
  //                    modelTransform[2] / scale.z, vec4(0.0f, 0.0f,
  //                    0.0f, 1.0f));
  //           selected->SetGlobalRotation(glm::quat_cast(rotation));
  //         } else if (mCurrentGizmoOperation == ImGuizmo::SCALE)
  //           selected->SetGlobalScale(scale);
  //       }
  //     }
  //   }
  // }
}
