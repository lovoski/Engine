// Keep in mind that multiple quaternion operations could be numerical unstable,
// use euler angles to keep record of continuous data, use quaternion for
// convenient rotation
#pragma once

#include "ecs/ecs.hpp"

class Transform : public ECS::BaseComponent {
public:
  vec3 Scale;
  // global rotation
  vec3 EulerAngles;
  const quat Rotation() { return glm::quat(EulerAngles); }
  // position under world axis
  vec3 Position;

  // scale relative to its parent's axis
  vec3 localScale = vec3(1.0f);
  // rotation relative to its parent's axis
  quat localRotation = quat(1.0f, vec3(0.0f));
  // position relative to its parent's axis
  vec3 localPosition = vec3(0.0f);

  static vec3 WorldUp, WorldRight, WorldForward;

  Transform(const vec3 translate = vec3(0.0f), const vec3 scale = vec3(1.0f),
            const vec3 angles = vec3(0.0f))
      : Position(translate), Scale(scale), EulerAngles(angles) {}

  ~Transform() {}

  mat4 GetModelMatrix() {
    mat4 model = mat4(1.0f);
    model = glm::translate(model, Position);
    model = model * glm::mat4_cast(Rotation());
    model = glm::scale(model, Scale);
    return model;
  }
};