#pragma once

#include "ecs/ecs.hpp"

class Transform : public ECS::BaseComponent {
public:
  vec3 Scale;
  // global rotation
  quat Rotation;
  // position under world axis
  vec3 Position;

  static vec3 WorldUp, WorldRight, WorldForward;

  Transform(const vec3 translate = vec3(0.0f), const vec3 scale = vec3(1.0f),
            const quat rotation = quat(1.0f, vec3(0.0f)))
      : Position(translate), Scale(scale), Rotation(rotation) {}

  ~Transform() {}

  mat4 GetModelMatrix() {
    mat4 model = mat4(1.0f);
    model = model * glm::mat4_cast(Rotation);
    model = glm::scale(model, Scale);
    model = glm::translate(model, Position);
    return model;
  }
};