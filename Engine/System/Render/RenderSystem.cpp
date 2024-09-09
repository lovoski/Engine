#include "System/Render/RenderSystem.hpp"
#include "Scene.hpp"

#include "Component/Camera.hpp"
#include "Component/Light.hpp"

#include "Function/Render/VisUtils.hpp"

namespace aEngine {

RenderSystem::RenderSystem() {
  Reset(); // initialize local variables
  AddComponentSignatureRequireAll<Mesh>();
  AddComponentSignatureRequireOne<MeshRenderer>();
  AddComponentSignatureRequireOne<DeformRenderer>();
}

void RenderSystem::bakeShadowMap() {
  for (int i = 0; i < Lights.size(); ++i) {
    auto light = Lights[i];
    light->StartShadow();
    shadowMapDirLight->Use();
    auto lightMatrix = light->GetShadowSpaceOrthoMatrix();
    shadowMapDirLight->SetMat4("LightSpaceMatrix", lightMatrix);
    if (light->type == LIGHT_TYPE::DIRECTIONAL_LIGHT) {
      // render shadow map for directional light
      auto offset =
          i * sizeof(LightData) + offsetof(LightData, meta) + 1 * sizeof(int);
      // set meta[1] to 1
      LightsBuffer.UpdateDataAs(GL_SHADER_STORAGE_BUFFER, 1, offset);
      offset = i * sizeof(LightData) + offsetof(LightData, lightMatrix);
      // set up lightMatrix
      LightsBuffer.UpdateDataAs(GL_SHADER_STORAGE_BUFFER, lightMatrix, offset);
      for (auto id : entities) {
        auto entity = GWORLD.EntityFromID(id);
        auto mesh = entity->GetComponent<Mesh>();
        // only perform rendering when the mesh is not null
        std::shared_ptr<MeshRenderer> renderer;
        if (entity->HasComponent<MeshRenderer>()) {
          renderer = entity->GetComponent<MeshRenderer>();
        } else if (entity->HasComponent<DeformRenderer>()) {
          renderer = entity->GetComponent<DeformRenderer>()->renderer;
          entity->GetComponent<DeformRenderer>()->DeformMesh(mesh);
        }
        if (renderer->castShadow) {
          renderer->DrawMeshShadowPass(*shadowMapDirLight, mesh,
                                       entity->GlobalTransformMatrix());
        }
      }
      offset = i * sizeof(LightData) + offsetof(LightData, shadowMapHandle);
      auto shadowMapHandle = glGetTextureHandleARB(light->ShadowMap);
      glMakeTextureHandleResidentARB(shadowMapHandle);
      // set up shadowMapHandle
      LightsBuffer.UpdateDataAs(GL_SHADER_STORAGE_BUFFER, shadowMapHandle,
                                offset);
    } else if (light->type == LIGHT_TYPE::POINT_LIGHT) {
      // render shadow map for point lights
    }
    light->EndShadow();
  }
  LightsBuffer.UnbindAs(GL_SHADER_STORAGE_BUFFER);
}

void RenderSystem::fillLightsBuffer() {
  std::vector<LightData> lightDataArray;
  for (auto light : Lights) {
    auto entity = GWORLD.EntityFromID(light->GetID());
    LightData ld;
    if (light->type == LIGHT_TYPE::DIRECTIONAL_LIGHT)
      ld.meta[0] = 0;
    else if (light->type == LIGHT_TYPE::POINT_LIGHT) {
      ld.meta[0] = 1;
      ld.fmeta[0] = light->lightRadius; // setup radius of point light
    }
    ld.meta[1] = 0;
    ld.color = glm::vec4(light->lightColor, 1.0f);
    ld.position = glm::vec4(entity->Position(), 1.0f);
    ld.direction = glm::vec4(entity->LocalForward, 1.0f);
    ld.lightMatrix = glm::mat4(1.0f);
    lightDataArray.push_back(ld);
  }
  LightsBuffer.SetDataAs(GL_SHADER_STORAGE_BUFFER, lightDataArray);
  LightsBuffer.UnbindAs(GL_SHADER_STORAGE_BUFFER);
}

void RenderSystem::ResizeAllShadowMaps() {
  for (auto light : Lights) {
    light->ResizeShadowMap(GlobalShadowMapSize, GlobalShadowMapSize);
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

    // Fill the LightsBuffer
    fillLightsBuffer();
    // Generate the shadow map
    if (EnableShadowMaps)
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
                              entity.get(), LightsBuffer);
    }

    // draw the grid in 3d space
    if (ShowGrid)
      VisUtils::DrawGrid(GridSize, GridSpacing, projMat * viewMat, GridColor);
  }
}

void RenderSystem::RenderEnd() { glfwSwapBuffers(GWORLD.Context.window); }

}; // namespace aEngine
