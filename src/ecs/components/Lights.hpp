#pragma once

#include "global.hpp"
#include "ecs/ecs.hpp"

class BaseLight : public ECS::BaseComponent {
public:

  enum LIGHT_TYPE {
    DIRECTIONAL_LIGHT,
    POINT_LIGHT,
    SPOT_LIGHT
  };

  BaseLight() {}
  BaseLight(LIGHT_TYPE type) : Type(type) {}
  ~BaseLight() {}

  LIGHT_TYPE Type = LIGHT_TYPE::DIRECTIONAL_LIGHT;

  // basic properties
  vec3 LightColor = vec3(0.5f);

  // directional light properties
  // the direction for a directional light is the localForward direction

  // point light properties

  // spot light properties
};