#pragma once

#include "FrameBuffer.hpp"
#include "EditorWindows.hpp"
#include "ecs/components/Material.hpp"
#include "ecs/components/MeshRenderer.hpp"
#include "ecs/components/Transform.hpp"
#include "ecs/components/Material.hpp"
#include "ecs/ecs.hpp"

class RenderSystem : public ECS::BaseSystem {
public:
  RenderSystem() {
    AddComponentSignature<Transform>();
    AddComponentSignature<MeshRenderer>();
  }
  ~RenderSystem() {}

  void Start() override {
    sceneBuffer =
        new Graphics::FrameBuffer(Core.VideoWdith(), Core.VideoHeight());
    // initialize the gui system
    editorWindows.Initialize();

    // create the default test scene
    ECS::EntityID camera;
    if (!editorWindows.GetActiveCamera(camera)) {
      ECS::Entity *cameraEntity = ECS::Manager.AddNewEntity();
      cameraEntity->name = "Main Camera";
      cameraEntity->AddComponent<Transform>(vec3(0.0f, 0.0f, 3.0f));
      cameraEntity->AddComponent<Camera>();
      editorWindows.SetActiveCamera(cameraEntity->ID);
    }
  }

  void Update() override {
    vec2 sceneAvailableSize = editorWindows.RenderStart(sceneBuffer);

    // adjust the size of framebuffer accordingly
    // glViewport(0, 0, sceneAvailableSize.x, sceneAvailableSize.y);
    // sceneBuffer->RescaleFrameBuffer(sceneAvailableSize.x, sceneAvailableSize.y);
    sceneBuffer->Bind();
    ECS::EntityID activeCamera;
    if (editorWindows.GetActiveCamera(activeCamera)) {
      // draw all the entities if there's an active camera

      for (auto entity : entities) {
        auto &renderer = ECS::Manager.GetComponent<MeshRenderer>(entity);
        auto &transform = ECS::Manager.GetComponent<Transform>(entity);
        // renderer.Render(activeCamera, transform);
      }
    } else {
      // clear all the channels if there's no active camera
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }
    sceneBuffer->Unbind();

    editorWindows.RenderComplete();
    glfwSwapBuffers(&Core.Window());
  }

  void Destroy() override {
    delete sceneBuffer;

  }

private:
  Graphics::FrameBuffer *sceneBuffer;
  EditorWindows editorWindows;
};