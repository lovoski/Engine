#pragma once

#include "Engine.hpp"
#include "Utils/Render/Shader.hpp"

using namespace aEngine;

struct EditorContext {
  // imguizmo settings
  ImGuizmo::OPERATION mCurrentGizmoOperation;
  ImGuizmo::MODE mCurrentGizmoMode;

  // context variable related to gui interactions
  EntityID selectedEntity = (EntityID)(-1);
  std::string activeBaseFolder = ".";
  const char *layoutFileName = "layout.ini";

  // imgui io
  ImGuiIO *io;

  void Reset() {
    mCurrentGizmoMode = ImGuizmo::MODE::WORLD;
    mCurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;

    selectedEntity = (EntityID)(-1);
    activeBaseFolder = ".";
    layoutFileName = "layout.ini";
  }
};

class Editor {
public:
  Editor(int width, int height);
  ~Editor() {
    if (engine)
      delete engine;
  }

  void Start();
  void Run(bool release = false);
  void Shutdown();

private:
  Engine *engine;

  unsigned int quadVAO, quadVBO;
  Render::Shader *quadShader;

  EditorContext context;

  void MainMenuBar();
  void EntitiesWindow();
  void ConsoleWindow();
  void AssetsWindow();
  void InspectorWindow();
  void DrawGizmos(float x, float y, float width, float height, bool enable = true);

};