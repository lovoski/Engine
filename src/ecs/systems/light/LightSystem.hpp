#pragma once

#include "ecs/components/Lights.hpp"
#include "ecs/components/Material.hpp"
#include "ecs/components/MeshRenderer.hpp"
#include "ecs/ecs.hpp"
#include "ecs/systems/render/RenderSystem.hpp"


class LightSystem : public ECS::BaseSystem {
public:
  LightSystem() {
    AddComponentSignature<Light>();
  }
  void Update() {
    // pass the information of all base lights to the render system
    auto rSystem = Core.EManager.GetSystemInstance<RenderSystem>();
    rSystem->activeBaseLights.clear();
    for (auto entity : entities) {
      rSystem->activeBaseLights.push_back(
          Core.EManager.GetComponent<Light>(entity));
    }
  }

private:
};