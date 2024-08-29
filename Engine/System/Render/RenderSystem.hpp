#pragma once

#include "Base/BaseSystem.hpp"

#include "Component/Light.hpp"
#include "Component/MeshRenderer.hpp"
#include "Component/DeformRenderer.hpp"

namespace aEngine {

class RenderSystem : public aEngine::BaseSystem {
public:
  RenderSystem() {
    Reset(); // initialize local variables
    AddComponentSignatureRequireOne<MeshRenderer>();
    AddComponentSignatureRequireOne<DeformRenderer>();
  }
  ~RenderSystem() {}

  void RenderBegin();
  void RenderEnd();

  void Reset() override {
    showGrid = true;
    gridSize = 10;
    gridSpacing = 1;
    gridColor = glm::vec3(0.5f);
    lights.clear();
  }

  // Grid options
  bool showGrid;
  unsigned int gridSize;
  unsigned int gridSpacing;
  glm::vec3 gridColor;

  std::vector<std::shared_ptr<Light>> lights;
};

}; // namespace aEngine
