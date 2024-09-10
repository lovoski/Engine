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
  for (auto id : entities) {
    GWORLD.GetComponent<Mesh>(id)->UpdateSpatialDS();
  }
}

void SpatialSystem::Update(float dt) {
  std::vector<std::shared_ptr<Entity>> e;
  for (auto i : entities)
    e.push_back(GWORLD.EntityFromID(i));
  for (int i = 0; i < entities.size(); ++i) {}
}

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