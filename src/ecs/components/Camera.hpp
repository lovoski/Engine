#pragma once

#include "global.hpp"
#include "ecs/ecs.hpp"
#include "ecs/components/Transform.hpp"

class Camera : public ECS::BaseComponent {
public:
  Camera() {}
  ~Camera() {}

  mat4 GetViewMatrix(Transform &transform) {
    vec3 forward = -(transform.Rotation * Transform::WorldForward);
    vec3 up = transform.Rotation * Transform::WorldUp;
    return glm::lookAt(transform.Position, transform.Position + forward, up);
  }

  mat4 GetProjMatrixPerspective(float width, float height) {
    return glm::perspective(fovY, width/height, zNear, zFar);
  }

  float fovY = 45.0f;
  float zNear = 0.1f, zFar = 100.0f;
};