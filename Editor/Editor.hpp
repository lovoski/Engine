#pragma once

#include "Engine.hpp"
#include "Function/Render/Shader.hpp"
#include "Function/Render/FrameBuffer.hpp"
#include "Function/AssetsLoader.hpp"

using namespace aEngine;

struct EditorContext {
  // imguizmo settings
  ImGuizmo::OPERATION mCurrentGizmoOperation;
  ImGuizmo::MODE mCurrentGizmoMode;

  // context variable related to gui interactions
  EntityID selectedEntity = (EntityID)(0);
  EntityID lockedSelectedEntity = (EntityID)(0);
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

    selectedEntity = (EntityID)(0);
    lockedSelectedEntity = (EntityID)(0);
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

  bool showEntitiesWindow = true;
  bool showAssetsWindow = true;
  bool showInspectorWindow = true;
  bool showImguiDemo = false;
  bool showImplotDemo = false;

  void MainMenuBar();
  void EntitiesWindow();
  void AssetsWindow();
  void InspectorWindow();
  void DrawGizmos(float x, float y, float width, float height, bool enable = true);

};