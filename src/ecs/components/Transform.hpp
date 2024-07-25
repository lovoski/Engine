#pragma once

#include "ecs/ecs.hpp"

class Transform : public ECS::BaseComponent {
public:
  vec3 Scale;
  // global rotation
  quat Rotation;
  // position under world axis
  vec3 Positions;

  static vec3 WorldUp, WorldForward, WorldRight;

  Transform(const vec3 translate = vec3(0.0f), const vec3 scale = vec3(1.0f),
            const quat rotation = quat(1.0f, vec3(0.0f)))
      : Positions(translate), Scale(scale), Rotation(rotation) {}

  ~Transform() {}

  mat4 GetModelMatrix() {
    mat4 model = mat4(1.0f);
    model = model * glm::mat4_cast(Rotation);
    model = glm::scale(model, Scale);
    model = glm::translate(model, Positions);
    return model;
  }
};

vec3 Transform::WorldUp = vec3(0.0f, 1.0f, 0.0f);
vec3 Transform::WorldRight = vec3(1.0f, 0.0f, 0.0f);
vec3 Transform::WorldForward = vec3(0.0f, 0.0f, 1.0f);