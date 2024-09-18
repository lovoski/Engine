#pragma once

#include "Base/BaseComponent.hpp"

#include "Function/AssetsType.hpp"

namespace aEngine {

struct Light : public aEngine::BaseComponent {
  Light(EntityID id) : BaseComponent(id) {}
  ~Light() {}

  bool enable = true;
  glm::vec3 LightColor = glm::vec3(1.0f);

  virtual glm::mat4 GetLightSpaceMatrix() { return glm::mat4(1.0f); }

  virtual void StartShadow() {}
  virtual void EndShadow() {}
};

// The direction of a directional light is LocalForward of its entity.
struct DirectionalLight : public Light {
  DirectionalLight(EntityID id);
  ~DirectionalLight();

  void ResizeShadowMap(unsigned int width, unsigned int height);

  void DrawInspectorGUI() override;

  glm::mat4 GetLightSpaceMatrix() override;

  void StartShadow() override;
  void EndShadow() override;

  unsigned int ShadowFBO, ShadowMap;
  unsigned int ShadowMapWidth = 1024, ShadowMapHeight = 1024;

  bool ShowShadowFrustom = false;

  float ShadowZNear = 0.1f, ShadowZFar = 10.0f;
  float ShadowOrthoW = 10.0f, ShadowOrthoH = 10.0f;

protected:
  int currentFBO;
  int viewport[4];
};

struct PointLight : public Light {
  PointLight(EntityID id);
  ~PointLight();

  void DrawInspectorGUI() override;

  float LightRadius = 0.5f;

protected:
};

struct EnvironmentLight : public Light {
  EnvironmentLight(EntityID id);
  ~EnvironmentLight();

  void DrawInspectorGUI() override;

  unsigned int CubeMap;

protected:
  int width = 1024, height = 1024;
  Texture faces[6], hdr;
  // this map is automatically build when cubemap is built
  unsigned int irradiance;
  void createCubeMapFromImages();
  void createCubeMapFromHDR();
};

}; // namespace aEngine
