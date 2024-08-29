#pragma once

#include "Base/Scriptable.hpp"
#include "Scene.hpp"
#include "Function/Render/VisUtils.hpp"

#include "Component/Camera.hpp"

namespace aEngine {

struct TestDebugDraw : public Scriptable {

  void DrawToScene() override {
    EntityID camera;
    if (GWORLD.GetActiveCamera(camera)) {
      auto cameraObject = GWORLD.EntityFromID(camera);
      auto cameraComp = cameraObject->GetComponent<Camera>();
      auto viewMat = cameraComp->ViewMat;
      auto projMat = cameraComp->ProjMat;
      auto vp = projMat * viewMat;
      // VisUtils::DrawLine3D(glm::vec3(0.0f), glm::vec3(2.0f), vp,
      //                      glm::vec3(1.0f, 0.0f, 0.0f));
      // VisUtils::DrawSquare(glm::vec3(0.0f), 1.0f, vp,
      //                      GWORLD.Context.sceneWindowSize,
      //                      glm::vec3(0.0f, 1.0f, 0.0f));
      // static std::vector<glm::vec3> lineStrip {
      //   glm::vec3(6.0f, 0.0f, 0.0f),
      //   glm::vec3(5.0f, 1.0f, 1.0f),
      //   glm::vec3(5.0f, 4.0f, 0.0f),
      //   glm::vec3(4.0f, 10.0f, 0.0f),
      // };
      // VisUtils::DrawLineStrip3D(lineStrip, vp, glm::vec3(0.0f, 1.0f, 1.0f));
    }
  }
};

}; // namespace aEngine