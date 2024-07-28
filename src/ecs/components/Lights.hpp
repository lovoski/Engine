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
  float LightColor[3] = {1.0f, 1.0f, 1.0f};

  // directional light properties
  vec3 LightDir = vec3(-1.0f);

  // point light properties

  // spot light properties
};