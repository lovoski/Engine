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
    auto mi = GWORLD.GetComponent<Mesh>(id);
    auto ei = GWORLD.EntityFromID(id);
    if (mi->AsCollider) {
      if (!mi->StaticCollider) {
        auto transform = ei->GlobalTransformMatrix();
        mi->BuildBVH(transform);
      }
      if (mi->bvh.Nodes().size() > 0) {
        // only check for collision when bvh is setup
        e.push_back(ei);
        m.push_back(mi);
      }
    }
  }
  for (int i = 0; i < e.size(); ++i) {
    for (int j = i + 1; j < e.size(); ++j) {
      std::vector<std::pair<int, int>> hit;
      if (m[i]->bvh.Intersect(m[j]->bvh, hit)) {
        LOG_F(INFO, "%d pairs of colliding triangle between \"%s\" and \"%s\"",
              hit.size(), e[i]->name.c_str(), e[j]->name.c_str());
      }
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
      if (mesh->bvh.Nodes().size() > 0 && mesh->AsCollider) {
        // draw bvh
        if (mesh->DrawLeafNodeOnly) {
          for (auto leafId : mesh->bvh.LeafNodes()) {
            auto &node = mesh->bvh.Nodes()[leafId];
            VisUtils::DrawAABB(node.bbox.Min, node.bbox.Max, vp);
          }
        } else {
          for (auto &node : mesh->bvh.Nodes()) {
            VisUtils::DrawAABB(node.bbox.Min, node.bbox.Max, vp);
          }
        }
      }
    }
  }
}

}; // namespace aEngine