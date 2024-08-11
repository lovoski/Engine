#pragma once

#include "Base/BaseComponent.hpp"

namespace aEngine {

struct Camera : public aEngine::BaseComponent {

  // The camera looks at -LocalForward direction
  glm::mat4 GetViewMatrix(Transform &transform) {
    return glm::lookAt(transform.Position(), transform.Position() - transform.LocalForward, transform.LocalUp);
  }

  glm::mat4 GetProjMatrixPerspective(float width, float height) {
    return glm::perspective(glm::radians(fovY), width/height, zNear, zFar);
  }

  float fovY = 45.0f, zNear = 0.1f, zFar = 100.0f;

};

};