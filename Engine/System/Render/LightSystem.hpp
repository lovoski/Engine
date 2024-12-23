#pragma once

#include "Base/BaseSystem.hpp"
#include "Scene.hpp"

#include "Component/Camera.hpp"
#include "Component/Light.hpp"
#include "System/Render/RenderSystem.hpp"

#include "Function/Render/VisUtils.hpp"

namespace aEngine {

struct LightData {
  // [0]: 0 for directional light, 1 for point light
  // [1]: 0 for not receive shadow, 1 for receive shadow
  int meta[4];
  // [0]: intensity of point light source
  float fmeta[4];
  glm::vec4 color;
  glm::vec4 position;    // for point light
  glm::vec4 direction;   // for directional light
  glm::mat4 lightMatrix; // light space transform matrix
};

class LightSystem : public aEngine::BaseSystem {
public:
  LightSystem();

  void Update(float dt) override;

  void Reset() override {
    activeSkyLight = nullptr;
    dlights.clear();
    plights.clear();
    skyLights.clear();
  }

  template <typename Archive>
  void serialize(Archive &ar) {
    ar(cereal::base_class<BaseSystem>(this));
  }

  // Draw visualizations for light sources
  void DebugRender();

  // The enabled global skylight
  std::vector<std::shared_ptr<EnvironmentLight>> skyLights;
  std::shared_ptr<EnvironmentLight> activeSkyLight = nullptr;
  // Stores pointers to all enabled lights
  std::vector<std::shared_ptr<DirectionalLight>> dlights;
  std::vector<std::shared_ptr<PointLight>> plights;
};

}; // namespace aEngine