#pragma once

#include "ecs/ecs.hpp"
#include "ecs/components/Transform.hpp"

class HierarchySystem : public ECS::BaseSystem {
public:
  HierarchySystem() {
    AddComponentSignature<Transform>();
  }
  ~HierarchySystem() {}

  void Update() {
    // clear old root entities
    hierarchyEntityRoots.clear();

    // update the local transform of all entities in hierarchy
    for (auto entity : entities) {
      auto entityObject = ECS::EManager.EntityFromID(entity);
      if (entityObject->parent == nullptr && entityObject->children.size() > 0) {
        hierarchyEntityRoots.push_back(entityObject);
      }
    }
  }

  // roots for hierarchies
  vector<ECS::Entity*> hierarchyEntityRoots;
private:

  void updateHierarchy(ECS::Entity *root) {
    // update the local transform component
  }
};