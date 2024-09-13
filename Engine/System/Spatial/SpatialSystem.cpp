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
  // // update the spatial ds, precompute the collision pair
  // std::vector<std::shared_ptr<Entity>> e;
  // std::vector<std::shared_ptr<Mesh>> m;
  // for (auto id : entities) {
  //   auto mi = GWORLD.GetComponent<Mesh>(id);
  //   if (mi->AsCollider) {
  //     auto ei = GWORLD.EntityFromID(id);

  //     auto transform = ei->GlobalTransformMatrix();
  //     mi->bvh.Clear();
  //     mi->SetupBVH(transform);

  //     e.push_back(ei);
  //     m.push_back(mi);
  //   } else {
  //     mi->bvh.Clear();
  //   }
  // }
  // static int counter = 0;
  // for (int i = 0; i < e.size(); ++i) {
  //   for (int j = i + 1; j < e.size(); ++j) {
  //     // test all pairs of faces
  //     auto transformi = e[i]->GlobalTransformMatrix();
  //     auto transformj = e[j]->GlobalTransformMatrix();
  //     bool collides = false;
  //     for (int fi = 0; fi < m[i]->bvh.GetNumPrimitives(); ++fi) {
  //       for (int fj = 0; fj < m[j]->bvh.GetNumPrimitives(); ++fj) {
  //         if (m[i]->bvh.Primitive(fi).Test(m[j]->bvh.Primitive(fj))) {
  //           LOG_F(
  //               INFO,
  //               "%d: Collision between face %d of \"%s\" and face %d of \"%s\"",
  //               counter, fi, e[i]->name.c_str(), fj, e[j]->name.c_str());
  //         }
  //       }
  //     }
  //   }
  // }
  // counter++;
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
      if (mesh->bvh.Nodes().size() > 0) {
        // draw bvh
        std::stack<int> s;
        s.push(0);
        while (!s.empty()) {
          auto cur = s.top();
          s.pop();

          auto &bbox = mesh->bvh.Nodes()[cur].bbox;
          VisUtils::DrawAABB(bbox.Min, bbox.Max, vp);

          auto lchild = mesh->bvh.Nodes()[cur].lchild;
          auto rchild = mesh->bvh.Nodes()[cur].rchild;
          if (lchild != -1)
            s.push(lchild);
          if (rchild != -1)
            s.push(rchild);
        }
      }
    }
  }
}

}; // namespace aEngine