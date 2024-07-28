#pragma once

#include "ecs/components/Lights.hpp"
#include "ecs/components/Material.hpp"
#include "ecs/components/MeshRenderer.hpp"
#include "ecs/components/Transform.hpp"
#include "ecs/ecs.hpp"
#include "ecs/systems/render/RenderSystem.hpp"


class BaseLightSystem : public ECS::BaseSystem {
public:
  BaseLightSystem() {
    AddComponentSignature<Transform>();
    AddComponentSignature<BaseLight>();
  }
  void Update() {
    // pass the information of all base lights to the render system
    auto rSystem = ECS::EManager.GetSystemInstance<RenderSystem>();
    rSystem->activeBaseLights.clear();
    for (auto entity : entities) {
      rSystem->activeBaseLights.push_back(
          ECS::EManager.GetComponent<BaseLight>(entity));
    }
  }

private:
};