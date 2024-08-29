/**
 * Maintain the activeLights property in GWORLD's context.
 */
#pragma once

#include "Base/BaseSystem.hpp"
#include "Scene.hpp"

#include "Component/Light.hpp"
#include "System/Render/RenderSystem.hpp"

namespace aEngine {

class LightSystem : public aEngine::BaseSystem {
public:
  LightSystem() { AddComponentSignatureRequireAll<Light>(); }

  void Update(float dt) override {
    auto renderSystem = GWORLD.GetSystemInstance<RenderSystem>();
    // Clear the light array
    renderSystem->lights.clear();
    for (auto id : entities) {
      if (GWORLD.EntityFromID(id)->Enabled)
        renderSystem->lights.push_back(GWORLD.GetComponent<Light>(id));
    }
  }

  // Draw visualizations for light sources
  void Render() {
    for (auto id : entities) {
      auto entity = GWORLD.EntityFromID(id);
      auto lightComp = entity->GetComponent<Light>();
      if (lightComp->type == LIGHT_TYPE::DIRECTIONAL_LIGHT) {
        
      } else if (lightComp->type == LIGHT_TYPE::POINT_LIGHT) {

      } else;
    }
  }
};

}; // namespace aEngine