#include "System/Render/RenderSystem.hpp"
#include "Scene.hpp"

#include "Component/Camera.hpp"
#include "Component/DeformMeshRenderer.hpp"
#include "Component/Light.hpp"
#include "Component/MeshRenderer.hpp"

#include "Utils/Render/VisUtils.hpp"

namespace aEngine {

RenderSystem::RenderSystem() {
  AddComponentSignatureRequireOne<MeshRenderer>();
  AddComponentSignatureRequireOne<DeformMeshRenderer>();
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
    glEnable(GL_DEPTH_TEST);
    for (auto entID : entities) {
      auto entity = GWORLD.EntityFromID(entID);
      if (entity->HasComponent<DeformMeshRenderer>()) {
        auto renderer = entity->GetComponent<DeformMeshRenderer>();
      } else if (entity->HasComponent<MeshRenderer>()) {
        auto renderer = entity->GetComponent<MeshRenderer>();
        renderer.ForwardRender(projMat, viewMat, camera, entity,
                             GWORLD.Context.activeLights);
      } else {
        Console.Log(
            "[error]: entity %s shouldn't be updated by render system\n",
            entity->name.c_str());
      }
      

    }

    // draw the grid in 3d space
    if (GWORLD.Context.showGrid)
      VisUtils::DrawGrid(GWORLD.Context.gridSize, projMat * viewMat,
                         GWORLD.Context.gridColor);
  }
}

void RenderSystem::RenderEnd() { glfwSwapBuffers(GWORLD.Context.window); }

}; // namespace aEngine
