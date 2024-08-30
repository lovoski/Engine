/**
 * Maintain the activeLights property in GWORLD's context.
 */
#pragma once

#include "Base/BaseSystem.hpp"
#include "Scene.hpp"

#include "Component/Camera.hpp"
#include "Component/Light.hpp"
#include "System/Render/RenderSystem.hpp"

#include "Function/Render/VisUtils.hpp"

namespace aEngine {

class LightSystem : public aEngine::BaseSystem {
public:
  LightSystem() { AddComponentSignatureRequireAll<Light>(); }

  void Update(float dt) override {
    auto renderSystem = GWORLD.GetSystemInstance<RenderSystem>();
    // Clear the light array
    renderSystem->Lights.clear();
    for (auto id : entities) {
      if (GWORLD.EntityFromID(id)->Enabled)
        renderSystem->Lights.push_back(GWORLD.GetComponent<Light>(id));
    }
  }

  // Draw visualizations for light sources
  void Render() {
    EntityID camera;
    if (GWORLD.GetActiveCamera(camera)) {
      auto cameraEntity = GWORLD.EntityFromID(camera);
      auto cameraComp = cameraEntity->GetComponent<Camera>();
      auto viewMat = cameraComp->ViewMat;
      auto projMat = cameraComp->ProjMat;
      for (auto id : entities) {
        auto entity = GWORLD.EntityFromID(id);
        auto lightComp = entity->GetComponent<Light>();
        if (lightComp->type == LIGHT_TYPE::DIRECTIONAL_LIGHT) {
          VisUtils::DrawDirectionalLight(entity->LocalForward, entity->LocalUp,
                                         entity->LocalLeft, entity->Position(),
                                         projMat * viewMat);
          VisUtils::DrawCube(
              entity->Position() -
                  (lightComp->ShadowOrthoW * 0.5f * entity->LocalLeft +
                   lightComp->ShadowOrthoH * 0.5f * entity->LocalUp -
                   lightComp->ShadowZNear * entity->LocalForward),
              entity->LocalForward, entity->LocalLeft, entity->LocalUp,
              projMat * viewMat, lightComp->ShadowZFar - lightComp->ShadowZNear,
              lightComp->ShadowOrthoW, lightComp->ShadowOrthoH);
        } else if (lightComp->type == LIGHT_TYPE::POINT_LIGHT) {
          VisUtils::DrawPointLight(entity->Position(), projMat * viewMat,
                                   lightComp->lightRadius);
        }
      }
    }
  }
};

}; // namespace aEngine