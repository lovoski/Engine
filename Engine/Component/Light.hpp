#pragma once

#include "Base/BaseComponent.hpp"

#include "Function/AssetsType.hpp"

namespace aEngine {

// The direction of a directional light is LocalForward of its entity.
struct DirectionalLight : public BaseComponent {
  DirectionalLight() : BaseComponent(0) {}
  DirectionalLight(EntityID id);
  ~DirectionalLight();

  void ResizeShadowMap(unsigned int width, unsigned int height);

  void DrawInspectorGUI() override;

  glm::mat4 GetLightSpaceMatrix();

  void StartShadow();
  void EndShadow();

  std::string getInspectorWindowName() override { return "Directional Light"; }

  template <typename Archive> void serialize(Archive &ar) {
    ar(CEREAL_NVP(entityID));
    ar(Enabled, LightColor, ShowShadowFrustom, ShadowZNear, ShadowZFar,
       ShadowOrthoW, ShadowOrthoH);
  }

  bool Enabled = true;
  glm::vec3 LightColor = glm::vec3(1.0f);

  unsigned int ShadowFBO, ShadowMap;
  unsigned int ShadowMapWidth = 1024, ShadowMapHeight = 1024;

  bool ShowShadowFrustom = false;

  float ShadowZNear = 0.1f, ShadowZFar = 10.0f;
  float ShadowOrthoW = 10.0f, ShadowOrthoH = 10.0f;

protected:
  int currentFBO;
  int viewport[4];
};

struct PointLight : public BaseComponent {
  PointLight() : BaseComponent(0) {}
  PointLight(EntityID id);
  ~PointLight();

  void DrawInspectorGUI() override;

  std::string getInspectorWindowName() override { return "Point Light"; }

  template <typename Archive> void serialize(Archive &ar) {
    ar(CEREAL_NVP(entityID));
    ar(Enabled, LightColor, LightRadius);
  }

  bool Enabled = true;
  glm::vec3 LightColor = glm::vec3(1.0f);

  float LightRadius = 0.5f;

protected:
};

struct EnvironmentLight : public BaseComponent {
  EnvironmentLight() : BaseComponent(0) {}
  EnvironmentLight(EntityID id);
  ~EnvironmentLight();

  void DrawInspectorGUI() override;

  std::string getInspectorWindowName() override { return "Environment Light"; }

  template <typename Archive> void serialize(Archive &ar) {
    ar(CEREAL_NVP(entityID));
    ar(Enabled, LightColor);
  }

  bool Enabled = true;
  glm::vec3 LightColor = glm::vec3(1.0f);

  unsigned int CubeMap;
  // This map is automatically computed when cubemap is built
  unsigned int Irradiance;

protected:
  int width = 1024, height = 1024;
  Texture faces[6], hdr;
  void createCubeMapFromImages();
  void createCubeMapFromHDR();
};

}; // namespace aEngine
