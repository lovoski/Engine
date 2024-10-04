#pragma once

#include "Base/BaseSystem.hpp"

#include "Component/DeformRenderer.hpp"
#include "Component/Light.hpp"
#include "Component/MeshRenderer.hpp"

#include "Function/Render/Buffers.hpp"

namespace aEngine {

class RenderSystem : public aEngine::BaseSystem {
public:
  RenderSystem();
  ~RenderSystem() {}

  void Render();
  void RenderEnd();

  void Reset() override {
    ShowGrid = true;
    GridSize = 10;
    GridSpacing = 1;
    GridColor = glm::vec3(0.5f);
    EnableShadowMap = false;
    ShadowMapResolution = 1024;
    shadowMapDirLight = Loader.GetShader("::shadowMapDirLight");
  }

  template <typename Archive>
  void serialize(Archive &ar) {
    ar(cereal::base_class<BaseSystem>(this));
    ar(ShowGrid, GridSize, GridSpacing, GridColor);
    ar(EnableShadowMap, ShadowMapResolution);
  }

  // Grid options
  bool ShowGrid;
  unsigned int GridSize;
  unsigned int GridSpacing;
  glm::vec3 GridColor;
  // Buffer for lights, hold an array of LightData
  Render::Buffer LightsBuffer;

  bool EnableShadowMap = false;
  int ShadowMapResolution = 1024;

  bool RenderSkybox = true;

protected:
  std::shared_ptr<Render::Shader> shadowMapDirLight;
  void bakeShadowMap();
};

}; // namespace aEngine