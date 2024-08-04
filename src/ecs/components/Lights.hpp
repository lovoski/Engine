#pragma once

#include "global.hpp"
#include "ecs/ecs.hpp"

class Light : public ECS::BaseComponent {
public:

  enum LIGHT_TYPE {
    DIRECTIONAL_LIGHT,
    POINT_LIGHT,
    SPOT_LIGHT
  };

  Light() {}
  Light(LIGHT_TYPE type) : Type(type) {}
  ~Light() {}

  LIGHT_TYPE Type = LIGHT_TYPE::DIRECTIONAL_LIGHT;

  // basic properties
  vec3 LightColor = vec3(0.5f);

  void Serialize(Json &json) override {
    json["Type"] = (int)Type;
    json["LightColor"] = LightColor;
  }

  void Deserialize(Json &json) override {
    Type = (LIGHT_TYPE)json["Type"];
    LightColor = json["LightColor"];
  }

  // directional light properties
  // the direction for a directional light is the localForward direction

  // point light properties

  // spot light properties
};