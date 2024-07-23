// entity is more a abstract concept
// each entity holds some reference to some components

// components are the actual structures storing the data

// systems update with the information in the entities have
// the same signature to it
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

  void Update() {
    // Update all the registered systems
    for (auto &system : registeredSystems) {
      system.second->Update();
    }
  }

  void Render() {
    for (auto &system : registeredSystems) {
      system.second->Render();
    }
  }

  const EntityID AddNewEntity() {
    const EntityID id = availableEntities.front();
    AddEntitySignature(id); // create a signature for the entity
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
    // the destroyed entity won't appear in any systems
    for (auto &system : registeredSystems) {
      system.second->RemoveEntity(entity);
    }
    entityCount--;
    availableEntities.push(entity);
  }

  template <typename T, typename... Args>
  void AddComponent(const EntityID entity, Args &&...args) {
    assert(entity < MAX_ENTITY_COUNT &&
           "EntityID out of range (MAX_ENTITY_COUNT) during AddComponent");
    assert(entitiesSignatures[entity].get()->size() < MAX_COMPONENT_COUNT &&
           "Component count limit reached (MAX_COMPONENT_COUNT)");

    // create the component with parameters
    T component(std::forward<Args>(args)...);
    component.entityID = entity;
    GetComponentList<T>()->Insert(component);

    // add the component to the very signature of this entity
    const ComponentTypeID compType = ComponentType<T>();
    // add the component to this entity's signature
    entitiesSignatures.at(entity).get()->insert(compType);
    // after adding the component, this entity could potentially
    // belong to some new systems
    UpdateEntityTargetSystems(entity);
  }

  template <typename T> void RemoveComponent(const EntityID entity) {
    assert(entity < MAX_ENTITY_COUNT &&
           "EntityID out of range (MAX_ENTITY_COUNT) during RemoveComponent");
    const ComponentTypeID compType = ComponentType<T>();
    // each entity has only one component of a specified componenet type
    entitiesSignatures.at(entity).get()->erase(compType);
    GetComponentList<T>()->Erase(entity);
    // after removing the component, this entity could no longer
    // beglong to some systems
    UpdateEntityTargetSystems(entity);
  }

  // find the component belongs to some entity
  template <typename T> T &GetComponent(const EntityID entity) {
    assert(entity < MAX_ENTITY_COUNT &&
           "EntityID out of range (MAX_ENTITY_COUNT) during GetComponent");
    const ComponentTypeID compType = ComponentType<T>();
    return GetComponentList<T>()->Get(entity);
  }

  // iterate through the signature of the entity to find the component with
  // indicated type
  template <typename T> const bool HasComponent(const EntityID entity) {
    assert(entity < MAX_ENTITY_COUNT &&
           "EntityID out of range (MAX_ENTITY_COUNT) during HasComponent");
    const EntitySignature signature = entitiesSignatures.at(entity);
    const ComponentTypeID compType = ComponentType<T>();
    auto it = std::find(signature.begin(), signature.end(), compType);
    return it != signature.end()
  }

  // register system at runtime
  template <typename T> void RegisterSystem() {
    const SystemTypeID systemType = SystemType<T>();
    assert(registeredSystems.count(systemType) == 0 &&
           "System already registered");
    auto system = std::make_shared<T>();
    // add entities that might belongs to the system
    for (EntityID entity = 0; entity < entityCount; ++entityCount) {
      AddEntityToSystem(entity, system.get());
    }
    system->Start();
    registeredSystems[systemType] = std::move(system);
  }

  template <typename T> void UnRegisterSystem() {
    const SystemTypeID systemType = SystemType<T>();
    assert(registeredSystems.count(systemType) != 0 && "System not registered");
    registeredSystems.erase(systemType);
  }

private:
  // create a component list that stores a specified type of components
  template <typename T> void AddComponentList() {
    const ComponentTypeID compType = ComponentType<T>();
    assert(componentsArrays.find(compType) == componentsArrays.end() &&
           "Component list already registered");
    componentsArrays[compType] =
        std::move(std::make_shared<ComponentList<T>>());
  }

  // get the component list storing the specified type of component
  template <typename T> std::shared_ptr<ComponentList<T>> GetComponentList() {
    const ComponentTypeID compType = ComponentType<T>();
    // get a component list that do not exists
    if (componentsArrays.count(compType) == 0) {
      AddComponentList<T>();
    }
    return std::static_pointer_cast<ComponentList<T>>(
        componentsArrays.at(compType));
  }

  void AddEntitySignature(const EntityID entity) {
    assert(entitiesSignatures.find(entity) == entitiesSignatures.end() &&
           "Signature not found");
    entitiesSignatures[entity] = std::move(std::make_shared<EntitySignature>());
  }

  std::shared_ptr<EntitySignature> GetEntitySignature(const EntityID entity) {
    // assert will be triggered when the condition is false
    assert(entitiesSignatures.find(entity) != entitiesSignatures.end() &&
           "Signature not found");
    return entitiesSignatures.at(entity);
  }

  // insert the entity to its systems
  // remove it from the system it don't belong to
  void UpdateEntityTargetSystems(const EntityID entity) {
    for (auto &system : registeredSystems) {
      AddEntityToSystem(entity, system.second.get());
    }
  }

  // add an entity to the system if it belongs to the system
  // if the entity don't belong to the system, erase it
  void AddEntityToSystem(const EntityID entity, BaseSystem *system) {
    if (BelongToSystem(entity, system->GetSignature())) {
      system->AddEntity(entity);
    } else {
      system->RemoveEntity(entity);
    }
  }

  // check if an entity belongs to the system by comparing their signatures
  // mind that an entity can have more components than the system's requirement
  bool BelongToSystem(const EntityID entity,
                      const EntitySignature &systemSignature) {
    // iterate through all the system by brute force
    for (const auto compType : systemSignature) {
      if (GetEntitySignature(entity)->count(compType) == 0) {
        return false;
      }
    }
    return true;
  }

  // how many entities have been created
  EntityID entityCount;
  std::queue<EntityID> availableEntities;
  // every time a entity is created, a signature for it will also be created
  std::map<EntityID, std::shared_ptr<EntitySignature>> entitiesSignatures;
  std::map<SystemTypeID, std::shared_ptr<BaseSystem>> registeredSystems;
  std::map<ComponentTypeID, std::shared_ptr<IComponentList>> componentsArrays;
};

}; // namespace ECS