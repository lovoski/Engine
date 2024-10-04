#include "System/Render/LightSystem.hpp"

namespace aEngine {

LightSystem::LightSystem() {
  Reset();
  AddComponentSignatureRequireOne<DirectionalLight>();
  AddComponentSignatureRequireOne<PointLight>();
  AddComponentSignatureRequireOne<EnvironmentLight>();
}

void LightSystem::Update(float dt) {
  std::vector<LightData> ld;
  dlights.clear();
  plights.clear();
  skyLights.clear();
  for (auto id : entities) {
    auto entity = GWORLD.EntityFromID(id);
    if (auto dirLight = GWORLD.GetComponent<DirectionalLight>(id)) {
      if (dirLight->Enabled) {
        LightData light;
        light.meta[0] = 0;
        light.color = glm::vec4(dirLight->LightColor, 1.0f);
        light.position = glm::vec4(entity->Position(), 1.0f);
        light.direction = glm::vec4(entity->LocalForward, 1.0f);
        ld.push_back(light);
        dlights.push_back(dirLight);
      }
    }
    if (auto pointLight = GWORLD.GetComponent<PointLight>(id)) {
      if (pointLight->Enabled) {
        LightData light;
        light.meta[0] = 1;
        light.fmeta[0] = pointLight->LightRadius;
        light.color = glm::vec4(pointLight->LightColor, 1.0f);
        light.position = glm::vec4(entity->Position(), 1.0f);
        ld.push_back(light);
        plights.push_back(pointLight);
      }
    }
    if (auto skyLightComp = GWORLD.GetComponent<EnvironmentLight>(id)) {
      if (skyLightComp->Enabled)
        skyLights.push_back(skyLightComp);
    }
  }
  auto renderSystem = GWORLD.GetSystemInstance<RenderSystem>();
  renderSystem->LightsBuffer.SetDataAs(GL_SHADER_STORAGE_BUFFER, ld);
  renderSystem->LightsBuffer.UnbindAs(GL_SHADER_STORAGE_BUFFER);
}

void LightSystem::Render() {
  EntityID camera;
  if (GWORLD.GetActiveCamera(camera)) {
    auto cameraEntity = GWORLD.EntityFromID(camera);
    auto cameraComp = cameraEntity->GetComponent<Camera>();
    auto viewMat = cameraComp->ViewMat;
    auto projMat = cameraComp->ProjMat;
    for (auto id : entities) {
      auto entity = GWORLD.EntityFromID(id);
      if (entity->HasComponent<DirectionalLight>()) {
        auto lightComp = entity->GetComponent<DirectionalLight>();
        VisUtils::DrawDirectionalLight(entity->LocalForward, entity->LocalUp,
                                       entity->LocalLeft, entity->Position(),
                                       projMat * viewMat);
        if (lightComp->ShowShadowFrustom) {
          VisUtils::DrawCube(
              entity->Position() -
                  (lightComp->ShadowOrthoW * 0.5f * entity->LocalLeft +
                   lightComp->ShadowOrthoH * 0.5f * entity->LocalUp -
                   lightComp->ShadowZNear * entity->LocalForward),
              entity->LocalForward, entity->LocalLeft, entity->LocalUp,
              projMat * viewMat,
              {lightComp->ShadowOrthoW, lightComp->ShadowOrthoH,
               lightComp->ShadowZFar - lightComp->ShadowZNear});
        }
      } else if (entity->HasComponent<PointLight>()) {
        auto lightComp = entity->GetComponent<PointLight>();
        VisUtils::DrawPointLight(entity->Position(), projMat * viewMat,
                                 lightComp->LightRadius);
      }
    }
  }
}

}; // namespace aEngine

REGISTER_SYSTEM(aEngine, LightSystem)