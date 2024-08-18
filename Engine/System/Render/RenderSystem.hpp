#pragma once

#include "Base/BaseSystem.hpp"

#include <functional>

namespace aEngine {

class RenderSystem : public aEngine::BaseSystem {
public:
  RenderSystem();
  ~RenderSystem();

  void RenderBegin();
  void RenderEnd();

};

}; // namespace aEngine
