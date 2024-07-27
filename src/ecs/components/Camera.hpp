#pragma once

#include "global.hpp"
#include "ecs/ecs.hpp"
#include "ecs/components/Transform.hpp"

class Camera : public ECS::BaseComponent {
public:
  Camera() {}
  ~Camera() {}

  enum MOVEMENT_STYLE {
    FPS_CAMERA
  };

  mat4 GetViewMatrix(Transform &transform) {
    vec3 forward = transform.Rotation() * Transform::WorldForward;
    vec3 up = transform.Rotation() * Transform::WorldUp;
    return glm::lookAt(transform.Position, transform.Position + forward, up);
  }

  mat4 GetProjMatrixPerspective(float width, float height) {
    return glm::perspective(glm::radians(fovY), width/height, zNear, zFar);
  }

  float fovY = 45.0f;
  float zNear = 0.1f, zFar = 100.0f;

  float movementSpeed = 2.5f, mouseSensitivity = 0.1f;
  MOVEMENT_STYLE moveScheme = MOVEMENT_STYLE::FPS_CAMERA;
};