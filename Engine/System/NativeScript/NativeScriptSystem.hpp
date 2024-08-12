#pragma once

#include "Base/BaseSystem.hpp"

namespace aEngine {

class NativeScriptSystem : public aEngine::BaseSystem {
public:
  NativeScriptSystem();
  ~NativeScriptSystem() {}

  void Update(float dt) override;
  void LateUpdate();

  void DrawToScene();

private:
};

};