#pragma once

#include "Base/BaseSystem.hpp"

namespace aEngine {

class RenderSystem : public aEngine::BaseSystem {
public:
  RenderSystem() {}
  ~RenderSystem() {}

  void RenderBegin();
  void RenderEnd();
};

};
