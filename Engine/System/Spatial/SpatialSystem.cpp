#include "System/Spatial/SpatialSystem.hpp"
#include "Entity.hpp"
#include "Scene.hpp"

#include "Function/Render/VisUtils.hpp"

#include "Component/Camera.hpp"
#include "Component/Mesh.hpp"

namespace aEngine {

SpatialSystem::SpatialSystem() {
  Reset();
  AddComponentSignatureRequireAll<Mesh>();
}

SpatialSystem::~SpatialSystem() {}

void SpatialSystem::PreUpdate(float dt) {
  // update the spatial ds, precompute the collision pair
  std::vector<std::shared_ptr<Entity>> e;
  std::vector<std::shared_ptr<Mesh>> m;
  for (auto id : entities) {
    e.push_back(GWORLD.EntityFromID(id));
    m.push_back(GWORLD.GetComponent<Mesh>(id));
    m.back()->UpdateSpatialDS();
  }
  for (int i = 0; i < e.size(); ++i) {
    for (int j = i + 1; j < e.size(); ++j) {
      // check for collision
    }
  }
}

void SpatialSystem::Update(float dt) {}

void SpatialSystem::Render() {
  EntityID camera;
  if (GWORLD.GetActiveCamera(camera)) {
    auto cameraComp = GWORLD.GetComponent<Camera>(camera);
    auto vp = cameraComp->VP;
    for (auto id : entities) {
      auto entity = GWORLD.EntityFromID(id);
      auto mesh = entity->GetComponent<Mesh>();
      // // draw AABB
      // VisUtils::DrawCube(mesh->bbox.Min, Entity::WorldForward,
      //                    Entity::WorldLeft, Entity::WorldUp, vp,
      //                    mesh->bbox.Max - mesh->bbox.Min);
    }
  }
}

}; // namespace aEngine