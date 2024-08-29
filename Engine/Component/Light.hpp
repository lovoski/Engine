#pragma once

#include "Base/BaseComponent.hpp"

namespace aEngine {

enum LIGHT_TYPE { DIRECTIONAL_LIGHT, POINT_LIGHT };

// The direction of DIRECTIONAL_LIGHT is LocalForward
struct Light : public aEngine::BaseComponent {
  LIGHT_TYPE type = LIGHT_TYPE::DIRECTIONAL_LIGHT;

  glm::vec3 lightColor = glm::vec3(0.5f);

  void DrawInspectorGUI() override;
};

}; // namespace aEngine
