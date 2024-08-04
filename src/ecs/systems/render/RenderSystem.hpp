#pragma once

#include "ecs/ecs.hpp"
#include "basics.hpp"

#include "ecs/components/Material.hpp"
#include "ecs/components/MeshRenderer.hpp"
#include "ecs/components/Lights.hpp"

class RenderSystem : public ECS::BaseSystem {
public:
  RenderSystem() {
    AddComponentSignature<MeshRenderer>();
  }
  ~RenderSystem() {}

  void Start() override {
    // enable opengl features
    glEnable(GL_DEPTH_TEST);
  }

  void Update() override {}

  void BeginRender() {
    Core.SceneBuffer->Bind();
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      Console.Log("[error]: Framebuffer is not complete\n");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    ECS::EntityID activeCamera;
    if (Core.GetActiveCamera(activeCamera)) {
      // draw all the entities if there's an active camera
      Camera cameraComp = Core.EManager.GetComponent<Camera>(activeCamera);
      Transform *camera = Core.EManager.EntityFromID(activeCamera);
      vec3 viewDir = -camera->LocalForward;
      mat4 viewMatrix = cameraComp.GetViewMatrix(*camera);
      mat4 projMatrix = cameraComp.GetProjMatrixPerspective(
          Core.SceneWindowSize.x, Core.SceneWindowSize.y);
      for (auto entity : entities) {
        auto &renderer = Core.EManager.GetComponent<MeshRenderer>(entity);
        auto transform = Core.EManager.EntityFromID(entity);
        Material *material =
            Core.EManager.HasComponent<Material>(entity)
                ? &Core.EManager.GetComponent<Material>(entity)
                : &defaultMaterial;
        renderer.Render(projMatrix, viewMatrix, camera, transform, material, activeBaseLights);
      }
    }
    Core.SceneBuffer->Unbind();
  }

  void EndRender() {
    glfwSwapBuffers(&Core.Window());
  }

  void Destroy() override {}

  // array storing all the information of active lights in the scene
  // this array is maintained by the light system
  vector<Light> activeBaseLights;

private:
  Material defaultMaterial;
};