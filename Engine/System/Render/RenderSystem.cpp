#include "System/Render/RenderSystem.hpp"
#include "Scene.hpp"

#include "Component/Camera.hpp"
#include "Component/Light.hpp"
#include "Component/Material.hpp"
#include "Component/MeshRenderer.hpp"

#include "Utils/Render/VisUtils.hpp"

namespace aEngine {

RenderSystem::RenderSystem() {
  AddComponentSignature<Material>();
  AddComponentSignature<MeshRenderer>();
}

RenderSystem::~RenderSystem() {}

void RenderSystem::RenderBegin() {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  EntityID cameraID;
  // draw the objects only with an active camera
  if (GWORLD.GetActiveCamera(cameraID)) {
    auto camera = GWORLD.EntityFromID(cameraID);
    auto cameraComp = camera->GetComponent<Camera>();
    glm::mat4 viewMat = cameraComp.GetViewMatrix(*camera);
    glm::mat4 projMat = cameraComp.GetProjMatrixPerspective(
        GWORLD.Context.sceneWindowSize.x, GWORLD.Context.sceneWindowSize.y);
    for (auto entID : entities) {
      auto entity = GWORLD.EntityFromID(entID);
      auto matComp = entity->GetComponent<Material>();
      auto renderer = entity->GetComponent<MeshRenderer>();
      renderer.ForwardRender(projMat, viewMat, camera, entity, &matComp,
                             GWORLD.Context.activeLights);
    }

    // draw the grid in 3d space
    if (GWORLD.Context.showGrid)
      VisUtils::DrawGrid(GWORLD.Context.gridSize, projMat * viewMat,
                         GWORLD.Context.gridColor);
  }
}

void RenderSystem::RenderEnd() { glfwSwapBuffers(GWORLD.Context.window); }

}; // namespace aEngine
