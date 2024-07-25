#pragma once

#include "ecs/ecs.hpp"
#include "object/Shader.hpp"

class Material : public ECS::BaseComponent {
public:
  Material() {}
  ~Material() {}

  static Material DefaultMaterial() {
    static Material defaultMaterial;
    return defaultMaterial;
  }

};