#include "System/Spatial/SpatialSystem.hpp"
#include "Entity.hpp"
#include "Scene.hpp"

#include "Component/Mesh.hpp"

namespace aEngine {

SpatialSystem::SpatialSystem() {
  Reset();
  AddComponentSignatureRequireAll<Mesh>();
}

SpatialSystem::~SpatialSystem() {}

void SpatialSystem::PreUpdate(float dt) {
  for (auto id : entities) {
    auto entity = GWORLD.EntityFromID(id);
  }
}

void SpatialSystem::Update(float dt) {}

void SpatialSystem::Render() {}

}; // namespace aEngine