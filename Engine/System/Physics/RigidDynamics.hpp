#pragma once

#include "Base/BaseSystem.hpp"
#include "Entity.hpp"
#include "Scene.hpp"

#include "Component/Phy/RigidBody.hpp"

namespace aEngine {

class RigidDynamics : public BaseSystem {
public:
  RigidDynamics();
  ~RigidDynamics();

  void PreUpdate(float dt) override;
  void Update(float dt) override;

  void DebugRender() override;
};

};