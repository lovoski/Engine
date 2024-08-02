#pragma once

#include "FrameBuffer.hpp"
#include "ecs/components/Material.hpp"
#include "ecs/components/MeshRenderer.hpp"
#include "ecs/components/Lights.hpp"
#include "ecs/ecs.hpp"
#include "basics.hpp"

class RenderSystem : public ECS::BaseSystem {
public:
  RenderSystem() {
    AddComponentSignature<MeshRenderer>();
  }
  ~RenderSystem() {}

  void Start() override {
    sceneBuffer =
        new Graphics::FrameBuffer(Core.VideoWdith(), Core.VideoHeight());
    // initialize the gui system
    EditorContext.Initialize();

    // enable opengl features
    glEnable(GL_DEPTH_TEST);
  }

  void Update() override {
    EditorContext.RenderStart(sceneBuffer);
    // adjust the size of framebuffer accordingly
    sceneBuffer->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    ECS::EntityID activeCamera;
    if (EditorContext.GetActiveCamera(activeCamera)) {
      // draw all the entities if there's an active camera
      Camera cameraComp = ECS::EManager.GetComponent<Camera>(activeCamera);
      Transform *camera = ECS::EManager.EntityFromID(activeCamera);
      vec3 viewDir = -camera->LocalForward;
      mat4 viewMatrix = cameraComp.GetViewMatrix(*camera);
      mat4 projMatrix = cameraComp.GetProjMatrixPerspective(
          EditorContext.SceneWindowSize.x, EditorContext.SceneWindowSize.y);
      for (auto entity : entities) {
        auto &renderer = ECS::EManager.GetComponent<MeshRenderer>(entity);
        auto transform = ECS::EManager.EntityFromID(entity);
        BaseMaterial *material =
            ECS::EManager.HasComponent<BaseMaterial>(entity)
                ? &ECS::EManager.GetComponent<BaseMaterial>(entity)
                : &defaultMaterial;
        renderer.Render(projMatrix, viewMatrix, camera, transform, material, activeBaseLights);
      }
    }
    sceneBuffer->Unbind();

    EditorContext.RenderComplete();
    glfwSwapBuffers(&Core.Window());
  }

  void Destroy() override { delete sceneBuffer; }

  // array storing all the information of active lights in the scene
  // this array is maintained by the light system
  vector<BaseLight> activeBaseLights;

private:
  Graphics::FrameBuffer *sceneBuffer;
  BaseMaterial defaultMaterial;
};