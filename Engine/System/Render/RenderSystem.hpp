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

  void SetRenderGUI(std::function<void(void)> guiRenderer) {
    guiLayer = guiRenderer;
  }

private:
  std::function<void(void)> guiLayer = []() {};
};

}; // namespace aEngine
