#pragma once

#include "Base/BaseSystem.hpp"

namespace aEngine {

class NativeScriptSystem : public aEngine::BaseSystem {
public:
  NativeScriptSystem();
  ~NativeScriptSystem() {}

  void Update(float dt) override;
  void LateUpdate(float dt);

  void DrawToScene();

  template <typename Archive>
  void serialize(Archive &ar) {
    ar(cereal::base_class<BaseSystem>(this));
  }
};

}; // namespace aEngine