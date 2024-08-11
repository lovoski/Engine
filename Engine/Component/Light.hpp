#pragma once

#include "Base/BaseComponent.hpp"

namespace aEngine {

enum LIGHT_TYPE {
  DIRECTIONAL_LIGHT,
  POINT_LIGHT,
  SPOT_LIGHT
};

struct Light : public aEngine::BaseComponent {
  LIGHT_TYPE type = LIGHT_TYPE::DIRECTIONAL_LIGHT;

  glm::vec3 lightColor = glm::vec3(0.5f);

  // directional light specific
  // the directional of light is LocalForward

  // point light specific
  // the position can be found at transform

  // spot light specific
};

};
