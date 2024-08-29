#pragma once

#include "Base/BaseComponent.hpp"

namespace aEngine {

enum LIGHT_TYPE { DIRECTIONAL_LIGHT, POINT_LIGHT };

// The direction of DIRECTIONAL_LIGHT is LocalForward
struct Light : public aEngine::BaseComponent {
  Light();
  ~Light();

  LIGHT_TYPE type = LIGHT_TYPE::DIRECTIONAL_LIGHT;

  glm::vec3 lightColor = glm::vec3(0.5f);

  void DrawInspectorGUI() override;

  unsigned int ShadowFBO, ShadowMap;
  unsigned int ShadowMapWidth = 1024, ShadowMapHeight = 1024;

  float ShadowZNear = 0.1f, ShadowZFar = 10.0f;
  float ShadowOrthoW = 10.0f, ShadowOrthoH = 10.0f;

  glm::mat4 GetShadowSpaceOrthoMatrix();

  void StartShadow();
  void EndShadow();

private:
  int currentFBO;
  int viewport[4];

};

}; // namespace aEngine
