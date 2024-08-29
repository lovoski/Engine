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

  void Render();
  void RenderEnd();

  void Reset() override {
    ShowGrid = true;
    GridSize = 10;
    GridSpacing = 1;
    GridColor = glm::vec3(0.5f);
    Lights.clear();
    EnableShadowMaps = true;
  }

  // Grid options
  bool ShowGrid;
  unsigned int GridSize;
  unsigned int GridSpacing;
  glm::vec3 GridColor;
  // Lights
  std::vector<std::shared_ptr<Light>> Lights;
  // Shadows
  bool EnableShadowMaps = true;
private:

  void bakeShadowMap();

};

}; // namespace aEngine
