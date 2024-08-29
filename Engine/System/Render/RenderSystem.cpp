#include "System/Render/RenderSystem.hpp"
#include "Scene.hpp"

#include "Component/Camera.hpp"
#include "Component/Light.hpp"

#include "Function/Render/VisUtils.hpp"

namespace aEngine {

void RenderSystem::RenderBegin() {
  glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  EntityID cameraID;
  // draw the objects only with an active camera
  if (GWORLD.GetActiveCamera(cameraID)) {
    auto camera = GWORLD.EntityFromID(cameraID);
    auto cameraComp = camera->GetComponent<Camera>();
    glm::mat4 viewMat = cameraComp->GetViewMatrix(*camera);
    glm::mat4 projMat = cameraComp->GetProjMatrixPerspective(
        GWORLD.Context.sceneWindowSize.x, GWORLD.Context.sceneWindowSize.y);
    glEnable(GL_DEPTH_TEST);
    for (auto entID : entities) {
      auto entity = GWORLD.EntityFromID(entID);
      if (entity->HasComponent<MeshRenderer>()) {
        auto &renderer = entity->GetComponent<MeshRenderer>();
        renderer->ForwardRender(projMat, viewMat, camera.get(), entity.get(),
                                GWORLD.Context.activeLights);
      } else if (entity->HasComponent<DeformRenderer>()) {
        auto &renderer = entity->GetComponent<DeformRenderer>();
        renderer->Render(projMat, viewMat, camera.get(), entity.get(),
                         GWORLD.Context.activeLights);
      }
    }

    // draw the grid in 3d space
    if (showGrid)
      VisUtils::DrawGrid(gridSize, gridSpacing, projMat * viewMat, gridColor);
  }
}

void RenderSystem::RenderEnd() { glfwSwapBuffers(GWORLD.Context.window); }

}; // namespace aEngine
