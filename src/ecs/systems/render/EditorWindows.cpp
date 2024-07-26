#include "EditorWindows.hpp"

void EditorWindows::Initialize() {
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
    Console.AddLog("[info]: what is this???\n");
  }
  auto entities = ECS::Manager.GetActiveEntities();
  for (auto i = 0; i < entities.size(); ++i) {
    if (ImGui::Selectable(entities[i]->name.c_str(), selectedEntityInd == i)) {
      selectedEntityInd = i;
      selectedEntity = entities[i]->ID;
    }
  }
  ImGui::End();
}

void EditorWindows::ConsoleWindow() {
  Console.Draw("Console");
}

void EditorWindows::AssetsWindow() {
  ImGui::Begin("Assets");
  ImGui::End();
}

vec2 EditorWindows::RenderStart(Graphics::FrameBuffer *sceneBuffer) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  MainMenuBar();

  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
  ImGui::Begin("Scene");
  ImGui::BeginChild("GameRenderer");
  auto size = ImGui::GetContentRegionAvail();
  ImGui::Image((void *)sceneBuffer->GetFrameTexture(), size);
  ImGui::EndChild();
  ImGui::End();

  // ImGui::ShowDemoWindow();

  EntitiesWindow();
  ConsoleWindow();
  ComponentsWindow();
  AssetsWindow();

  return vec2(size.x, size.y);
}

void EditorWindows::RenderComplete() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
