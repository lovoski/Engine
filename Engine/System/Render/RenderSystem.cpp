#include "Scene.hpp"
#include "System/Render/RenderSystem.hpp"

namespace aEngine {

void RenderSystem::RenderBegin() {
  sceneManager->Context.frameBuffer->Bind();
  sceneManager->Context.frameBuffer->Unbind();
}

void RenderSystem::RenderEnd() {}

};
