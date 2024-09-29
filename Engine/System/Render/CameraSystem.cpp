#include "System/Render/CameraSystem.hpp"

namespace aEngine {

void CameraSystem::PreUpdate(float dt) {
  for (auto id : entities) {
    auto entity = GWORLD.EntityFromID(id);
    auto cameraComp = entity->GetComponent<Camera>();
    cameraComp->GetCameraViewPerpProjMatrix(cameraComp->ViewMat,
                                            cameraComp->ProjMat);
    cameraComp->VP = cameraComp->ProjMat * cameraComp->ViewMat;
  }
}

void CameraSystem::Render() {
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

std::vector<std::shared_ptr<Entity>> CameraSystem::GetAvailableCamera() {
  std::vector<std::shared_ptr<Entity>> result;
  for (auto id : entities) {
    auto entity = GWORLD.EntityFromID(id);
    result.push_back(entity);
  }
  return result;
}

}; // namespace aEngine

BOOST_CLASS_EXPORT(aEngine::CameraSystem)