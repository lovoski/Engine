#pragma once

#include "FrameBuffer.hpp"
#include "basics.hpp"
#include "ecs/components/Camera.hpp"
#include "ecs/components/Material.hpp"
#include "ecs/components/MeshRenderer.hpp"
#include "ecs/components/Transform.hpp"
#include "global.hpp"

class EditorWindows {
public:
  EditorWindows() {}
  ~EditorWindows() {}

  void Initialize() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImFontConfig fontConfig;
    fontConfig.SizePixels = 20.0f;
    io.Fonts->AddFontDefault(&fontConfig);

    io.IniFilename = layoutFileName;

    ImGui_ImplGlfw_InitForOpenGL(&Core.Window(), true);
    ImGui_ImplOpenGL3_Init("#version 450");
  }
  void Destroy() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
  }

  // Define the gui layout, returns the available size for scene rendering
  vec2 RenderStart(Graphics::FrameBuffer *sceneBuffer) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

    ImGui::Begin("Scene");
    ImGui::BeginChild("GameRenderer");
    auto size = ImGui::GetContentRegionAvail();
    ImGui::Image((void *)sceneBuffer->GetFrameTexture(), size);
    ImGui::EndChild();
    ImGui::End();

    ImGui::Begin("Entities");
    if (ImGui::Button("add entity", ImVec2(-1, 60))) {
      ECS::Manager.AddNewEntity();
    }
    auto entities = ECS::Manager.GetActiveEntities();
    vector<const char*> listBoxItems;
    for (auto i = 0; i < entities.size(); ++i) {
      listBoxItems.push_back(entities[i]->name.c_str());
    }
    int currentIndex = 0;
    ImGui::ListBox("active entities", &currentIndex, listBoxItems.data(), entities.size());
    ImGui::End();
    return vec2(size.x, size.y);
  }

  void RenderComplete() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }

  bool GetActiveCamera(ECS::EntityID &camera) {
    if (hasActiveCamera) {
      camera = activeCamera;
      return true;
    } else {
      cout << "There's no active camera" << endl;
      camera = (ECS::EntityID)(-1);
      return false;
    }
  }

  // The camera entity must has a transform component and a camera component
  bool SetActiveCamera(ECS::EntityID camera) {
    if (ECS::Manager.HasComponent<Camera>(camera) &&
        ECS::Manager.HasComponent<Transform>(camera)) {
      hasActiveCamera = true;
      activeCamera = camera;
      return true;
    } else {
      cout << "Not a valid camera entity" << endl;
      // there could exist an active camera,
      // don't reset the hasActiveCamera flag
      return false;
    }
  }

private:
  const char *layoutFileName = "layout.ini";
  bool hasActiveCamera = false;
  ECS::EntityID activeCamera;
};