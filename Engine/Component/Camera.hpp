#pragma once

#include "Base/BaseComponent.hpp"
#include "Entity.hpp"

namespace aEngine {

struct Camera : public aEngine::BaseComponent {
  Camera() : BaseComponent(-1) {}
  Camera(EntityID id) : BaseComponent(id) {}

  // The camera look at -LocalForward direction
  void GetCameraViewPerpProjMatrix(glm::mat4 &view, glm::mat4 &proj);

  void DrawInspectorGUI() override;

  std::string getInspectorWindowName() override { return "Camera"; }

  template <typename Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &boost::serialization::base_object<BaseComponent>(*this);
    ar &fovY;
    ar &zFar;
    ar &zNear;
  }

  float fovY = 45.0f, zNear = 0.1f, zFar = 100.0f;

  glm::mat4 ViewMat, ProjMat, VP;
};

}; // namespace aEngine