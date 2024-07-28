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

    // enable opengl features
    glEnable(GL_DEPTH_TEST);

    // create the default test scene
    ECS::Entity *cameraEntity = ECS::EManager.AddNewEntity();
    cameraEntity->name = "Main Camera";
    cameraEntity->AddComponent<Transform>(vec3(0.0f, 0.0f, 6.0f), vec3(1.0f), vec3(0.0f, glm::radians(180.0f), 0.0f));
    cameraEntity->AddComponent<Camera>();
    EditorContext.SetActiveCamera(cameraEntity->ID);

    auto cubeObject = ECS::EManager.AddNewEntity();
    cubeObject->name = "cube primitive";
    cubeObject->AddComponent<Transform>(vec3(3.0f, 0.0f, 0.0f));
    cubeObject->AddComponent<MeshRenderer>(
        Resource::RManager.GetPrimitive(
            Resource::PRIMITIVE_TYPE::CUBE));
    cubeObject->AddComponent<BaseMaterial>();

    auto sphereObject = ECS::EManager.AddNewEntity();
    sphereObject->name = "sphere primitive";
    sphereObject->AddComponent<Transform>(vec3(-3.0f, 0.0f, 0.0f));
    sphereObject->AddComponent<MeshRenderer>(
        Resource::RManager.GetPrimitive(
            Resource::PRIMITIVE_TYPE::SPHERE));
    sphereObject->AddComponent<BaseMaterial>();

    auto planeObject = ECS::EManager.AddNewEntity();
    planeObject->name = "plane primitive";
    planeObject->AddComponent<Transform>(vec3(0.0f, -3.0f, 0.0f), vec3(10.0f, 1.0f, 5.0f));
    planeObject->AddComponent<MeshRenderer>(
        Resource::RManager.GetPrimitive(
            Resource::PRIMITIVE_TYPE::PLANE));
    planeObject->AddComponent<BaseMaterial>();

    auto loadedMeshes = Resource::RManager.GetModel(REPO_SOURCE_DIR "/assets/learnopengl/objects/backpack/backpack.obj");
    for (auto mesh : loadedMeshes) {
      auto nObject = ECS::EManager.AddNewEntity();
      nObject->AddComponent<Transform>(vec3(0.0f), vec3(0.5f));
      nObject->AddComponent<MeshRenderer>(mesh);
    }
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
      Transform cameraTrans =
          ECS::EManager.GetComponent<Transform>(activeCamera);
      mat4 viewMatrix = cameraComp.GetViewMatrix(cameraTrans);
      mat4 projMatrix = cameraComp.GetProjMatrixPerspective(
          EditorContext.SceneWindowSize.x, EditorContext.SceneWindowSize.y);
      for (auto entity : entities) {
        auto &renderer = ECS::EManager.GetComponent<MeshRenderer>(entity);
        auto &transform = ECS::EManager.GetComponent<Transform>(entity);
        BaseMaterial *material =
            ECS::EManager.HasComponent<BaseMaterial>(entity)
                ? &ECS::EManager.GetComponent<BaseMaterial>(entity)
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