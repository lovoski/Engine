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

  // Maintain the transform matrices for all cameras
  void PreUpdate(float dt) override {
    for (auto id : entities) {
      auto entity = GWORLD.EntityFromID(id);
      auto cameraComp = entity->GetComponent<Camera>();
      cameraComp->GetCameraViewPerpProjMatrix(cameraComp->ViewMat, cameraComp->ProjMat);
      cameraComp->VP = cameraComp->ProjMat * cameraComp->ViewMat;
    }
  }

  void Render() {}

  std::vector<std::shared_ptr<Entity>> GetAvailableCamera() {
    std::vector<std::shared_ptr<Entity>> result;
    for (auto id : entities) {
      auto entity = GWORLD.EntityFromID(id);
      result.push_back(entity);
    }
    return result;
  }
};

}; // namespace aEngine