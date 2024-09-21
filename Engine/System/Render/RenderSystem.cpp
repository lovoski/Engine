#include "System/Render/RenderSystem.hpp"
#include "Scene.hpp"
#include "System/Render/LightSystem.hpp"

#include "Component/Camera.hpp"
#include "Component/Light.hpp"

#include "Function/Render/Tools.hpp"
#include "Function/Render/VisUtils.hpp"

namespace aEngine {

RenderSystem::RenderSystem() {
  Reset(); // initialize local variables
  AddComponentSignatureRequireAll<Mesh>();
  AddComponentSignatureRequireOne<MeshRenderer>();
  AddComponentSignatureRequireOne<DeformRenderer>();
}

void RenderSystem::bakeShadowMap() {
  auto lightSystem = GWORLD.GetSystemInstance<LightSystem>();
  auto &lights = lightSystem->lights;
  LightsBuffer.BindAs(GL_SHADER_STORAGE_BUFFER);
  for (int i = 0; i < lights.size(); ++i) {
    auto light = lights[i];
    light->StartShadow();
    if (auto dirLight = dynamic_cast<DirectionalLight *>(light.get())) {
      shadowMapDirLight->Use();
      auto lightMatrix = dirLight->GetLightSpaceMatrix();
      shadowMapDirLight->SetMat4("LightSpaceMatrix", lightMatrix);
      auto offset =
          i * sizeof(LightData) + offsetof(LightData, meta) + 1 * sizeof(int);
      // this light will cast shadow
      LightsBuffer.UpdateDataAs(GL_SHADER_STORAGE_BUFFER, 1, offset);
      offset = i * sizeof(LightData) + offsetof(LightData, lightMatrix);
      // setup light matrix
      LightsBuffer.UpdateDataAs(GL_SHADER_STORAGE_BUFFER, lightMatrix, offset);
      for (auto id : entities) {
        auto entity = GWORLD.EntityFromID(id);
        auto mesh = entity->GetComponent<Mesh>();
        std::shared_ptr<MeshRenderer> renderer;
        if (auto r = entity->GetComponent<MeshRenderer>()) {
          renderer = r;
        } else if (auto d = entity->GetComponent<DeformRenderer>()) {
          renderer = d->renderer;
          d->DeformMesh(mesh);
        }
        if (renderer->castShadow) {
          renderer->DrawMeshShadowPass(*shadowMapDirLight, mesh,
                                       entity->GlobalTransformMatrix());
        }
      }
      // bindless texture setup
      auto shadowMapHandle = glGetTextureHandleARB(dirLight->ShadowMap);
      glMakeTextureHandleResidentARB(shadowMapHandle);
      offset = i * sizeof(LightData) + offsetof(LightData, shadowMapHandle);
      LightsBuffer.UpdateDataAs(GL_SHADER_STORAGE_BUFFER, shadowMapHandle,
                                offset);
    } else if (auto pointLight = dynamic_cast<PointLight *>(light.get())) {
    }
    light->EndShadow();
  }
  LightsBuffer.UnbindAs(GL_SHADER_STORAGE_BUFFER);
}

void RenderSystem::Render() {
  auto lightSystem = GWORLD.GetSystemInstance<LightSystem>();

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

    if (RenderSkybox && (lightSystem->activeSkyLight != nullptr))
      Render::RenderEnvironmentMap(lightSystem->activeSkyLight->CubeMap,
                                   projMat * glm::mat4(glm::mat3(viewMat)));

    if (EnableShadowMap)
      bakeShadowMap();

    // The main render pass
    for (auto entityID : entities) {
      auto entity = GWORLD.EntityFromID(entityID);
      auto mesh = entity->GetComponent<Mesh>();

      std::shared_ptr<MeshRenderer> renderer;
      if (entity->HasComponent<DeformRenderer>()) {
        renderer = entity->GetComponent<DeformRenderer>()->renderer;
        entity->GetComponent<DeformRenderer>()->DeformMesh(mesh);
      } else if (entity->HasComponent<MeshRenderer>()) {
        renderer = entity->GetComponent<MeshRenderer>();
      }
      renderer->ForwardRender(mesh, projMat, viewMat, camera.get(),
                              entity.get(), LightsBuffer,
                              lightSystem->activeSkyLight);
    }

    // draw the grid in 3d space
    if (ShowGrid)
      VisUtils::DrawGrid(GridSize, GridSpacing, projMat * viewMat, GridColor);
  }
}

void RenderSystem::RenderEnd() {
  glfwSwapBuffers(GWORLD.Context.window);
}

}; // namespace aEngine
