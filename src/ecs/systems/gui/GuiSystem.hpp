#pragma once

#include "ecs/ecs.hpp"

class GuiSystem : public ECS::BaseSystem {
public:
  GuiSystem() {}
  ~GuiSystem() {}

  void Start() override;
  void Render() override;
  void Destroy() override;

  // imgui variables
  ImGuiIO *io = nullptr;

private:

  // window settings
  bool showEntityWindow = true;
  bool showAssetsWindow = true;
  bool showConsoleWindow = true;
  bool showInspectorWindow = true;

  void MainMenuBar();
  void EntitiesWindow();
  void ConsoleWindow();
  void AssetsWindow();
  void InspectorWindow();
  void DrawGizmos(float x, float y, float width, float height, bool enable = true);

  // imguizmo settings
  bool showGizmoGrid = true;
  int gizmoGridSize = 10;
  ImGuizmo::OPERATION mCurrentGizmoOperation;
  ImGuizmo::MODE mCurrentGizmoMode;

  // context variable related to gui interactions
  ECS::EntityID selectedEntity = -1;
  string activeBaseFolder = ".";
  const char *layoutFileName = "layout.ini";
};