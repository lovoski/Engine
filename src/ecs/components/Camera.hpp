#pragma once

#include "global.hpp"
#include "ecs/ecs.hpp"

class Camera : public ECS::BaseComponent {
  SerializableType(Camera);
public:
  Camera() {}
  ~Camera() {}

  enum MOVEMENT_STYLE {
    FPS_CAMERA
  };

  // The camera looks at -LocalForward direction
  mat4 GetViewMatrix(Transform &transform) {
    return glm::lookAt(transform.Position(), transform.Position() - transform.LocalForward, transform.LocalUp);
  }

  mat4 GetProjMatrixPerspective(float width, float height) {
    return glm::perspective(glm::radians(fovY), width/height, zNear, zFar);
  }

  float fovY = 45.0f;
  SerializableField(fovY);
  float zNear = 0.1f, zFar = 100.0f;
  SerializableField(zNear);
  SerializableField(zFar);

  float movementSpeed = 2.5f, mouseSensitivity = 0.1f;
  SerializableField(movementSpeed);
  SerializableField(mouseSensitivity);

  MOVEMENT_STYLE moveScheme = MOVEMENT_STYLE::FPS_CAMERA;
};