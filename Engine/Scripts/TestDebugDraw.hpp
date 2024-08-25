#pragma once

#include "Base/Scriptable.hpp"
#include "Scene.hpp"
#include "Function/Render/VisUtils.hpp"

#include "Component/Camera.hpp"

namespace aEngine {

struct TestDebugDraw : public Scriptable {

  void DrawToScene() override {
    auto scene = entity->scene;
    EntityID camera;
    if (scene->GetActiveCamera(camera)) {
      auto cameraObject = scene->EntityFromID(camera);
      auto cameraComp = cameraObject->GetComponent<Camera>();
      auto viewMat = cameraComp->GetViewMatrix(*cameraObject);
      auto projMat = cameraComp->GetProjMatrixPerspective(
          scene->Context.sceneWindowSize.x, scene->Context.sceneWindowSize.y);
      VisUtils::DrawLine3D(glm::vec3(0.0f), glm::vec3(2.0f), projMat * viewMat,
                           glm::vec3(1.0f, 0.0f, 0.0f));
      VisUtils::DrawSquare(glm::vec3(0.0f), 1.0f, projMat * viewMat,
                           scene->Context.sceneWindowSize,
                           glm::vec3(0.0f, 1.0f, 0.0f));
    }
  }
};

}; // namespace aEngine