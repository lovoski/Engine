#pragma once

#include "Engine.hpp"
#include "Utils/Render/Shader.hpp"
#include "System/Render/FrameBuffer.hpp"

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

  // The framebuffer storing scene's rendering
  FrameBuffer *frameBuffer;

  float editorLastRender, editorDeltaRender;

  static EditorContext &Ref() {
    static EditorContext context;
    return context;
  }

  // Reset editor context,
  // this function should get called when reloading the scene
  void Reset() {
    mCurrentGizmoMode = ImGuizmo::MODE::WORLD;
    mCurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;

    selectedEntity = (EntityID)(-1);
    activeBaseFolder = ".";
    layoutFileName = "layout.ini";

    editorLastRender = 0.0f;
    editorDeltaRender = 0.0f;
  }
};

// Make editor context a singleton
static EditorContext &context = EditorContext::Ref();

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

  void MainMenuBar();
  void MainSequencer();
  void EntitiesWindow();
  void ConsoleWindow();
  void AssetsWindow();
  void InspectorWindow();
  void DrawGizmos(float x, float y, float width, float height, bool enable = true);

};