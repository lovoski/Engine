#pragma once

#include "Entity.hpp"
#include "Base/BaseComponent.hpp"

namespace aEngine {

struct Camera : public aEngine::BaseComponent {

  // The camera look at -LocalForward direction
  void GetCameraViewPerpProjMatrix(glm::mat4 &view, glm::mat4 &proj) {
    auto size = GWORLD.Context.sceneWindowSize;
    auto entity = GWORLD.EntityFromID(entityID);
    proj = glm::perspective(glm::radians(fovY), size.x / size.y, zNear, zFar);
    view = glm::lookAt(entity->Position(), entity->Position() - entity->LocalForward, entity->LocalUp);
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

  glm::mat4 ViewMat, ProjMat, VP;

};

};