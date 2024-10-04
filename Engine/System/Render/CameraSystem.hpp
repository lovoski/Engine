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
  void PreUpdate(float dt) override;

  void Render();

  std::vector<std::shared_ptr<Entity>> GetAvailableCamera();

  template <typename Archive>
  void serialize(Archive &ar) {
    ar(cereal::base_class<BaseSystem>(this));
  }
};

}; // namespace aEngine