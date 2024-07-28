#pragma once

#include "ecs/components/Transform.hpp"
#include "ecs/ecs.hpp"


class HierarchySystem : public ECS::BaseSystem {
public:
  HierarchySystem() { AddComponentSignature<Transform>(); }
  ~HierarchySystem() {}

  void Start() {
    orderedEntities.reserve(ECS::MAX_ENTITY_COUNT);
    hierarchyEntityRoots.reserve(ECS::MAX_ENTITY_COUNT);
  }

  void Update() {
    // rebuild hierarchy
    RebuildHierarchyStructure();
    RecomputeGlobalTransformWithLocalTransform();
  }

  void RecomputeGlobalTransformWithLocalTransform() {
    // update the local transform of all entities in hierarchy
    for (auto entity : orderedEntities) {
      if (entity->parent != nullptr) {
        // TODO: fill in
      }
    }
  }

  void RebuildHierarchyStructure() {
    orderedEntities.clear();
    hierarchyEntityRoots.clear();
    std::queue<ECS::Entity*> q;
    for (auto entity : entities) {
      auto entityObject = ECS::EManager.EntityFromID(entity);
      if (entityObject->parent == nullptr) {
        q.push(entityObject);
        orderedEntities.push_back(entityObject);
        hierarchyEntityRoots.push_back(entityObject);
      }
    }
    // bfs
    while (!q.empty()) {
      auto ent = q.front();
      q.pop();
      for (auto child : ent->children) {
        q.push(child);
        orderedEntities.push_back(child);
      }
    }
  }

  void AttachNewChildToParent(ECS::Entity *newChild, ECS::Entity *parent) {
    auto r1 = std::find(parent->children.begin(), parent->children.end(), newChild);
    if (r1 != parent->children.end()) {
      Console.Log("[info]: entity %d is already the child of entity %d\n", newChild->ID, parent->ID);
      return;
    }
    // keep the global transform of the new child
    // TODO: fill in

    // connect the parent and child
    if (newChild->parent != nullptr) {
      bool findAsChild = false;
      for (auto child : newChild->parent->children)
        if (child->ID == newChild->ID) findAsChild = true;
      if (!findAsChild) {
        Console.Log("[error]: entity %d is not a child of its parent %d\n", newChild->ID, newChild->parent->ID);
        return;
      }
      auto it = std::find(newChild->parent->children.begin(), newChild->parent->children.end(), newChild);
      // remove the new child from its old parent
      newChild->parent->children.erase(it);
    }
    newChild->parent = parent;
    parent->children.push_back(newChild);
    RebuildHierarchyStructure();
  }

  // ordered entities, when it's time to update some entity, it's parent has
  // always been updated
  vector<ECS::Entity *> orderedEntities;
  // roots for hierarchies
  vector<ECS::Entity *> hierarchyEntityRoots;

};