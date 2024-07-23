#pragma once

#include "base_component.hpp"
#include "base_system.hpp"
#include "component_list.hpp"
#include "types.hpp"

namespace ECS {

class EntityManager {
public:
  EntityManager() : entityCount(0) {
    for (EntityID entity = 0u; entity < MAX_ENTITY_COUNT; ++entity) {
      availableEntities.push(entity);
    }
  }
  ~EntityManager() {}

  const EntityID AddNewEntity() {
    const EntityID id = availableEntities.front();
    availableEntities.pop();
    entityCount++;
    return id; // this entity can now access some components
  }

  void DestroyEntity(const EntityID entity) {
    assert(entity < MAX_ENTITY_COUNT && "Destroying entity out of range");
    entitiesSignatures.erase(entity);
    for (auto &array : componentsArrays) {
      array.second->Erase(entity);
    }
    for (auto &system : registeredSystems) {
      system.second->RemoveEntity(entity);
    }
    entityCount--;
    availableEntities.push(entity);
  }

private:
  // how many entities have been created
  EntityID entityCount;
  std::queue<EntityID> availableEntities;
  // every time a entity is created, a signature for it will also be created
  std::map<EntityID, EntitySignature> entitiesSignatures;
  std::map<SystemTypeID, std::unique_ptr<BaseSystem>> registeredSystems;
  std::map<ComponentTypeID, std::shared_ptr<IComponentList>> componentsArrays;
};

}; // namespace ECS