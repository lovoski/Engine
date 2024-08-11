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
  sceneManager->Context.frameBuffer->Bind();
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  // // layer 1: background
  // glDisable(GL_DEPTH_TEST);
  // glEnable(GL_DEPTH_TEST);

  // layer 2: objects
  EntityID cameraID;
  // draw the objects only with an active camera
  if (sceneManager->GetActiveCamera(cameraID)) {
    auto camera = sceneManager->EntityFromID(cameraID);
    auto cameraComp = camera->GetComponent<Camera>();
    glm::mat4 viewMat = cameraComp.GetViewMatrix(*camera);
    glm::mat4 projMat = cameraComp.GetProjMatrixPerspective(
        sceneManager->Context.sceneWindowSize.x,
        sceneManager->Context.sceneWindowSize.y);
    for (auto entID : entities) {
      auto entity = sceneManager->EntityFromID(entID);
      auto matComp = entity->GetComponent<Material>();
      auto renderer = entity->GetComponent<MeshRenderer>();
      renderer.ForwardRender(projMat, viewMat, camera, entity, &matComp,
                             sceneManager->Context.activeLights);
    }

    // draw the grid in 3d space
    if (sceneManager->Context.showGrid)
      VisUtils::DrawGrid(sceneManager->Context.gridSize,
                         sceneManager->Context.gridColor, projMat * viewMat);
  }

  // // layer 3: on top
  // glDisable(GL_DEPTH_TEST);
  // this->guiLayer();
  // glEnable(GL_DEPTH_TEST);
  sceneManager->Context.frameBuffer->Unbind();
}

void RenderSystem::RenderEnd() {
  glfwSwapBuffers(sceneManager->Context.window);
}

}; // namespace aEngine
