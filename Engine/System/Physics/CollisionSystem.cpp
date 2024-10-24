#include "System/Physics/CollisionSystem.hpp"
#include "Entity.hpp"
#include "Scene.hpp"

#include "Function/Render/VisUtils.hpp"

#include "Component/Camera.hpp"
#include "Component/Mesh.hpp"
#include "Component/Phy/Collider.hpp"
#include "Component/Phy/RigidBody.hpp"

namespace aEngine {

CollisionSystem::CollisionSystem() {
  Reset();
  AddComponentSignatureRequireOne<CubeCollider>();
}

CollisionSystem::~CollisionSystem() {}

void CollisionSystem::PreUpdate(float dt) {
  // perform broad phase colision update
}

void CollisionSystem::Update(float dt) {}

void CollisionSystem::DebugRender() {
  EntityID camera;
  if (GWORLD.GetActiveCamera(camera)) {
    auto cameraComp = GWORLD.GetComponent<Camera>(camera);
    auto vp = cameraComp->VP;
    auto viewport = GWORLD.Context.sceneWindowSize;
  }
}

}; // namespace aEngine

REGISTER_SYSTEM(aEngine, CollisionSystem)