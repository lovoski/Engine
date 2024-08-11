#pragma once

#include "Base/BaseSystem.hpp"

namespace aEngine {

class NativeScriptSystem : public aEngine::BaseSystem {
public:
  NativeScriptSystem();
  ~NativeScriptSystem() {}

  void Update() override;
  void LateUpdate() override;

  void DrawToScene();

private:
};

};