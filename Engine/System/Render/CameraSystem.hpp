/**
 * Maintain the View and Projection matrix of all cameras on the scene
 */
#pragma once

#include "Base/BaseSystem.hpp"
#include "Scene.hpp"

#include "Component/Camera.hpp"

namespace aEngine {

class CameraSystem : public BaseSystem {
public:
  CameraSystem() { AddComponentSignatureRequireAll<Camera>(); }

  void Update(float dt) override {
    for (auto id : entities) {
      auto entity = GWORLD.EntityFromID(id);
      auto cameraComp = entity->GetComponent<Camera>();
      cameraComp->ViewMat = cameraComp->GetViewMatrix(*entity);
      cameraComp->ProjMat = cameraComp->GetProjMatrixPerspective(
          GWORLD.Context.sceneWindowSize.x, GWORLD.Context.sceneWindowSize.y);
      cameraComp->VP = cameraComp->ProjMat * cameraComp->ViewMat;
    }
  }

  void Render() {}
};

}; // namespace aEngine