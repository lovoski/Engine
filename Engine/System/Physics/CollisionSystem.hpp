#pragma once

#include "Base/BaseSystem.hpp"
#include "Entity.hpp"
#include "Scene.hpp"

namespace aEngine {

class CollisionSystem : public BaseSystem {
public:
  CollisionSystem();
  ~CollisionSystem();

  void Reset() override {}

  void PreUpdate(float dt) override;
  void Update(float dt) override;

  // Render debug utils related to geometry
  void DebugRender() override;

  template <typename Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<BaseSystem>(this));
  }
};

}; // namespace aEngine