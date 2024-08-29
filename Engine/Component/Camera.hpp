#pragma once

#include "Entity.hpp"
#include "Base/BaseComponent.hpp"

namespace aEngine {

struct Camera : public aEngine::BaseComponent {

  // The camera looks at -LocalForward direction
  glm::mat4 GetViewMatrix(Entity &transform) {
    return glm::lookAt(transform.Position(), transform.Position() - transform.LocalForward, transform.LocalUp);
  }

  glm::mat4 GetProjMatrixPerspective(float width, float height) {
    return glm::perspective(glm::radians(fovY), width/height, zNear, zFar);
  }

  void DrawInspectorGUI() override {
    if (ImGui::TreeNode("Camera")) {
      ImGui::DragFloat("FovY", &fovY, 1.0f, 0.0f, 150.0f);
      ImGui::DragFloat("zNear", &zNear, 0.001f, 0.0000001f, 10.0f);
      ImGui::DragFloat("zFar", &zFar, 0.1f, 20.0f, 2000.0f);
      ImGui::TreePop();
    }
  }

  float fovY = 45.0f, zNear = 0.1f, zFar = 100.0f;

  // This variable is maintained by CameraSystem and gets updated
  // in CameraSystem's Update function
  glm::mat4 ViewMat, ProjMat, VP;

};

};