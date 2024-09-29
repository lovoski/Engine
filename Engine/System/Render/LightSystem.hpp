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
  // bindless handle for shadow map,
  // requires `ARB_bindless_texture` extension
  int64_t shadowMapHandle[2];
};

class LightSystem : public aEngine::BaseSystem {
public:
  LightSystem();

  void Update(float dt) override;

  void Reset() override {
    activeSkyLight = nullptr;
    lights.clear();
    skyLights.clear();
  }

  // template <typename Archive> void save(Archive &archive) const {
  //   // save the ids of entity posessing the light
  //   std::vector<EntityID> e1, e2;
  //   EntityID e3 = activeSkyLight == nullptr ? -1 : activeSkyLight->GetID();
  //   for (auto &id : skyLights)
  //     e1.push_back(id->GetID());
  //   for (auto &id : lights)
  //     e2.push_back(id->GetID());
  //   archive(cereal::base_class<BaseSystem>(this));
  //   archive(e1, e2, e3);
  // }

  // template <typename Archive> void load(Archive &archive) {
  //   std::vector<EntityID> e1, e2;
  //   EntityID e3;
  //   archive(cereal::base_class<BaseSystem>(this));
  //   archive(e1, e2, e3);
  //   Reset();
  //   for (auto id : e1)
  //     skyLights.push_back(GWORLD.GetComponent<EnvironmentLight>(id));
  //   for (auto id : e2)
  //     if (auto dlight = GWORLD.GetComponent<DirectionalLight>(id))
  //       lights.push_back(dlight);
  //     else if (auto plight = GWORLD.GetComponent<PointLight>(id))
  //       lights.push_back(plight);
  //   if (e3 != (EntityID)(-1)) {
  //     activeSkyLight = GWORLD.GetComponent<EnvironmentLight>(e3);
  //   }
  // }

  // Draw visualizations for light sources
  void Render();

  // The enabled global skylight
  std::vector<std::shared_ptr<EnvironmentLight>> skyLights;
  std::shared_ptr<EnvironmentLight> activeSkyLight = nullptr;
  // Stores pointers to all enabled lights
  std::vector<std::shared_ptr<Light>> lights;
};

}; // namespace aEngine