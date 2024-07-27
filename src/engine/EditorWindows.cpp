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
  auto entities = ECS::Manager.GetActiveEntities();
  for (auto i = 0; i < entities.size(); ++i) {
    if (ImGui::Selectable(entities[i]->name.c_str(), selectedEntityInd == i)) {
      selectedEntityInd = i;
      selectedEntity = entities[i]->ID;
    }
  }
  ImGui::End();
}

void EditorWindows::ConsoleWindow() { Console.Draw("Console"); }

void EditorWindows::AssetsWindow() {
  ImGui::Begin("Assets");
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
  ImGui::Image((void *)sceneBuffer->GetFrameTexture(), size, ImVec2(1, 1), ImVec2(0, 0));
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
