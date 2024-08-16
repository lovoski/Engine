/**
 * Maintain the activeLights property in GWORLD's context.
 */
#pragma once

#include "Base/BaseSystem.hpp"
#include "Component/Light.hpp"
#include "Scene.hpp"


namespace aEngine {

class LightSystem : public aEngine::BaseSystem {
public:
  LightSystem() { AddComponentSignatureRequireAll<Light>(); }

  void Update(float dt) override {
    GWORLD.Context.activeLights.clear();
    for (auto id : entities) {
      if (GWORLD.EntityFromID(id)->Enabled)
        GWORLD.Context.activeLights.push_back(GWORLD.GetComponent<Light>(id));
    }
  }

private:
};

}; // namespace aEngine