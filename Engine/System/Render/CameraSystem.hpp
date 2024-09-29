/**
 * Maintain the View and Projection matrix of all cameras on the scene
 */
#pragma once

#include "Base/BaseSystem.hpp"
#include "Scene.hpp"

#include "Component/Camera.hpp"

#include "Function/Render/VisUtils.hpp"

namespace aEngine {

class CameraSystem : public BaseSystem {
public:
  CameraSystem() { AddComponentSignatureRequireAll<Camera>(); }

  // Maintain the transform matrices for all cameras
  void PreUpdate(float dt) override {
    for (auto id : entities) {
      auto entity = GWORLD.EntityFromID(id);
      auto cameraComp = entity->GetComponent<Camera>();
      cameraComp->GetCameraViewPerpProjMatrix(cameraComp->ViewMat,
                                              cameraComp->ProjMat);
      cameraComp->VP = cameraComp->ProjMat * cameraComp->ViewMat;
    }
  }

  void Render() {
    EntityID camera;
    if (GWORLD.GetActiveCamera(camera)) {
      auto activeCameraEntity = GWORLD.EntityFromID(camera);
      auto activeCameraComp = activeCameraEntity->GetComponent<Camera>();
      float aspect =
          GWORLD.Context.sceneWindowSize.x / GWORLD.Context.sceneWindowSize.y;
      for (auto id : entities) {
        auto entity = GWORLD.EntityFromID(id);
        auto cameraComp = entity->GetComponent<Camera>();
        VisUtils::DrawCamera(entity->LocalForward, entity->LocalUp,
                             entity->LocalLeft, entity->Position(),
                             activeCameraComp->VP, cameraComp->fovY, aspect);
      }
    }
  }

  std::vector<std::shared_ptr<Entity>> GetAvailableCamera() {
    std::vector<std::shared_ptr<Entity>> result;
    for (auto id : entities) {
      auto entity = GWORLD.EntityFromID(id);
      result.push_back(entity);
    }
    return result;
  }

  template <typename Archive> void serialize(Archive &archive) {
  }
};

}; // namespace aEngine