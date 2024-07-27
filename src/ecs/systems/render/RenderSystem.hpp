#pragma once

#include "FrameBuffer.hpp"
#include "ecs/components/Material.hpp"
#include "ecs/components/MeshRenderer.hpp"
#include "ecs/components/Transform.hpp"
#include "ecs/ecs.hpp"
#include "basics.hpp"

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
    EditorContext.Initialize();
    // initialize the defualt material
    defaultMaterial.SetShader();

    // enable opengl features
    glEnable(GL_DEPTH_TEST);

    // create the default test scene
    ECS::Entity *cameraEntity = ECS::Manager.AddNewEntity();
    cameraEntity->name = "Main Camera";
    cameraEntity->AddComponent<Transform>(vec3(0.0f, 0.0f, 3.0f), vec3(1.0f), quat(0.0f, vec3(0.0f, 1.0f, 0.0f)));
    cameraEntity->AddComponent<Camera>();
    EditorContext.SetActiveCamera(cameraEntity->ID);

    auto defaultObject = ECS::Manager.AddNewEntity();
    defaultObject->AddComponent<Transform>();
    defaultObject->AddComponent<MeshRenderer>(
        Resource::ResourceManager.GetPrimitive(
            Resource::PRIMITIVE_TYPE::SPHERE));
  }

  void Update() override {
    EditorContext.RenderStart(sceneBuffer);
    // adjust the size of framebuffer accordingly
    sceneBuffer->Bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    ECS::EntityID activeCamera;
    if (EditorContext.GetActiveCamera(activeCamera)) {
      // draw all the entities if there's an active camera
      Camera cameraComp = ECS::Manager.GetComponent<Camera>(activeCamera);
      Transform cameraTrans =
          ECS::Manager.GetComponent<Transform>(activeCamera);
      mat4 viewMatrix = cameraComp.GetViewMatrix(cameraTrans);
      mat4 projMatrix = cameraComp.GetProjMatrixPerspective(
          EditorContext.SceneWindowSize.x, EditorContext.SceneWindowSize.y);
      for (auto entity : entities) {
        auto &renderer = ECS::Manager.GetComponent<MeshRenderer>(entity);
        auto &transform = ECS::Manager.GetComponent<Transform>(entity);
        BaseMaterial *material =
            ECS::Manager.HasComponent<BaseMaterial>(entity)
                ? &ECS::Manager.GetComponent<BaseMaterial>(entity)
                : &defaultMaterial;
        renderer.Render(projMatrix, viewMatrix, transform, material);
      }
    }
    sceneBuffer->Unbind();

    EditorContext.RenderComplete();
    glfwSwapBuffers(&Core.Window());
  }

  void Destroy() override { delete sceneBuffer; }

private:
  Graphics::FrameBuffer *sceneBuffer;
  BaseMaterial defaultMaterial;
};