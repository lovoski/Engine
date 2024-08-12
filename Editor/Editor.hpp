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

  static EditorContext &Ref() {
    static EditorContext context;
    return context;
  }

  void Reset() {
    mCurrentGizmoMode = ImGuizmo::MODE::WORLD;
    mCurrentGizmoOperation = ImGuizmo::OPERATION::TRANSLATE;

    selectedEntity = (EntityID)(-1);
    activeBaseFolder = ".";
    layoutFileName = "layout.ini";
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

  unsigned int quadVAO, quadVBO;
  Render::Shader *quadShader;

  void MainMenuBar();
  void EntitiesWindow();
  void ConsoleWindow();
  void AssetsWindow();
  void InspectorWindow();
  void DrawGizmos(float x, float y, float width, float height, bool enable = true);

};