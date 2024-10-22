#include "System/Physics/RigidDynamics.hpp"

namespace aEngine {

RigidDynamics::RigidDynamics() {
  Reset();
  AddComponentSignatureRequireAll<RigidBody>();
}

RigidDynamics::~RigidDynamics() {}

void RigidDynamics::Update(float dt) {}

void RigidDynamics::PreUpdate(float dt) {}

void RigidDynamics::DebugRender() {}

}; // namespace aEngine

REGISTER_SYSTEM(aEngine, RigidDynamics)