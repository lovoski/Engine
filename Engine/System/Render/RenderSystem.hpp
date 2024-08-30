#pragma once

#include "Base/BaseSystem.hpp"

#include "Component/DeformRenderer.hpp"
#include "Component/Light.hpp"
#include "Component/MeshRenderer.hpp"

#include "Function/Render/Buffers.hpp"

namespace aEngine {

struct LightData {
  // [0]: 0 for directional light, 1 for point light
  // [1]: 0 for not receive shadow, 1 for receive shadow
  int meta[4];
  glm::vec4 color;
  glm::vec4 position; // for point light
  glm::vec4 direction; // for directional light
  glm::mat4 lightMatrix; // light space transform matrix
};

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
    shadowMapDirLight = Loader.GetShader("::shadowMapDirLight");
  }

  // Grid options
  bool ShowGrid;
  unsigned int GridSize;
  unsigned int GridSpacing;
  glm::vec3 GridColor;
  // Lights, maintained by LightSystem
  std::vector<std::shared_ptr<Light>> Lights;
  // Buffer for lights, hold an array of LightData
  Render::Buffer LightsBuffer;
  // Shadows
  bool EnableShadowMaps = true;

private:
  void bakeShadowMap();
  void fillLightsBuffer();

  Render::Shader *shadowMapDirLight;
};

}; // namespace aEngine
