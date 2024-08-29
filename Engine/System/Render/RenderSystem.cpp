#include "System/Render/RenderSystem.hpp"
#include "Scene.hpp"

#include "Component/Camera.hpp"
#include "Component/Light.hpp"

#include "Function/Render/VisUtils.hpp"

namespace aEngine {

void RenderSystem::bakeShadowMap() {
  for (auto light : Lights) {
    light->StartShadow();
    shadowMapDirLight->Use();
    shadowMapDirLight->SetMat4("LightSpaceMatrix",
                               light->GetShadowSpaceOrthoMatrix());
    for (auto id : entities) {
      auto entity = GWORLD.EntityFromID(id);
      std::shared_ptr<MeshRenderer> renderer;
      if (entity->HasComponent<MeshRenderer>()) {
        renderer = entity->GetComponent<MeshRenderer>();
      } else if (entity->HasComponent<DeformRenderer>()) {
        renderer = entity->GetComponent<DeformRenderer>()->renderer;
      }
      shadowMapDirLight->SetMat4("Model", entity->GetModelMatrix());
      renderer->DrawMesh(*shadowMapDirLight);
    }
    light->EndShadow();
  }
}

void RenderSystem::Render() {
  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  EntityID cameraID;
  // draw the objects only with an active camera
  if (GWORLD.GetActiveCamera(cameraID)) {
    auto camera = GWORLD.EntityFromID(cameraID);
    auto cameraComp = camera->GetComponent<Camera>();
    glm::mat4 viewMat = cameraComp->ViewMat;
    glm::mat4 projMat = cameraComp->ProjMat;
    glEnable(GL_DEPTH_TEST);
    // Generate the shadow map
    if (EnableShadowMaps)
      bakeShadowMap();
    // The main render pass
    for (auto entityID : entities) {
      auto entity = GWORLD.EntityFromID(entityID);
      std::shared_ptr<MeshRenderer> renderer;
      if (entity->HasComponent<MeshRenderer>()) {
        renderer = entity->GetComponent<MeshRenderer>();
      } else if (entity->HasComponent<DeformRenderer>()) {
        renderer = entity->GetComponent<DeformRenderer>()->renderer;
        entity->GetComponent<DeformRenderer>()->DeformMesh();
      }
      renderer->ForwardRender(projMat, viewMat, camera.get(), entity.get(),
                              Lights);
    }

    // draw the grid in 3d space
    if (ShowGrid)
      VisUtils::DrawGrid(GridSize, GridSpacing, projMat * viewMat, GridColor);
  }
}

void RenderSystem::RenderEnd() { glfwSwapBuffers(GWORLD.Context.window); }

}; // namespace aEngine
