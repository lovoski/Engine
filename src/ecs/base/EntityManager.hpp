// entity is more a abstract concept
// each entity holds some reference to some components

// components are the actual structures storing the data

// systems update with the information in the entities have
// the same signature to it
#pragma once

#include "BaseComponent.hpp"
#include "BaseSystem.hpp"
#include "ComponentList.hpp"
#include "Types.hpp"

namespace ECS {

class EntityManager {
public:
  class Entity {
  public:

    friend class EntityManager;

    Entity(EntityID id, EntityManager *manager) : ID(id), MGR(manager) {
      m_scale = vec3(1.0f);
      m_position = vec3(0.0f);
      m_eulerAngles = vec3(0.0f);
    }
    ~Entity() {
      if (parent != nullptr) {
        // remove this child from its parent's child list
        auto it = std::find(parent->children.begin(), parent->children.end(), this);
        if (it != parent->children.end())
          parent->children.erase(it);
        parent = nullptr;
      }
      children.clear();
    }

    // global scale
    const vec3 Scale() { return m_scale; };
    // global rotation (pitch, yaw, roll) = (x, y, z)
    // in radians
    const vec3 EulerAngles() { return m_eulerAngles; }
    // euler angles in degree
    const vec3 EulerAnglesDegree() { return glm::degrees(m_eulerAngles); }
    // global rotation
    const quat Rotation() { return glm::quat(m_eulerAngles); }
    // position under world axis
    const vec3 Position() { return m_position; }

    // scale relative to its parent's axis
    vec3 localScale = vec3(1.0f);
    // rotation relative to its parent's axis
    quat localRotation = quat(1.0f, vec3(0.0f));
    // position relative to its parent's axis
    vec3 localPosition = vec3(0.0f);

    static vec3 WorldUp, WorldLeft, WorldForward;

    // local axis are updated at the start of each loop
    vec3 LocalUp, LocalLeft, LocalForward;

    // set global position
    void SetGlobalPosition(vec3 p) {
      // change global position, modify local position to satisfy the global
      // position 
      m_position = p;
      // the local axis will be updated in GlobalToLocal function call
      localPosition = GlobalToLocal(p);
    }
    // set global rotation (pitch, yaw, roll) = (x, y, z)
    // in degrees
    void SetGlobalRotationDegree(vec3 a) {
      a = glm::radians(a);
      SetGlobalRotation(a);
    }
    // set global rotation (pitch, yaw, roll) = (x, y, z)
    // in radians
    void SetGlobalRotation(vec3 a) {
      quat parentOrien = GetParentOrientation();
      localRotation = glm::inverse(parentOrien) * quat(a);
      m_eulerAngles = a;
      UpdateLocalAxis();
    }
    // set global rotation
    void SetGlobalRotation(quat q) {
      quat parentOrien = GetParentOrientation();
      localRotation = glm::inverse(parentOrien) * q;
      m_eulerAngles = glm::eulerAngles(q);
      UpdateLocalAxis();
    }
    // set global scale
    void SetGlobalScale(vec3 s) {
      localScale = s / GetParentScale();
      m_scale = s;
    }

    void UpdateLocalAxis() {
      quat q = GetParentOrientation() * localRotation;
      LocalForward = q * WorldForward;
      LocalLeft = q * WorldLeft;
      LocalUp = q * WorldUp;
    }

    // global position to the local position relative to its parent
    const vec3 GlobalToLocal(vec3 globalPos) {
      vec3 pLocalForward, pLocalLeft, pLocalUp;
      GetParentLocalAxis(pLocalForward, pLocalLeft, pLocalUp);
      // mat3 M_p = mat3(pLocalLeft.x, pLocalUp.x, pLocalForward.x,
      //                 pLocalLeft.y, pLocalUp.y, pLocalForward.y,
      //                 pLocalLeft.z, pLocalUp.z, pLocalForward.z);
      // mat3 M = mat3(WorldLeft.x, WorldUp.x, WorldForward.x,
      //               WorldLeft.y, WorldUp.y, WorldForward.y,
      //               WorldLeft.z, WorldUp.z, WorldForward.z);
      // TODO: always remembers how to intiailize a matrix
      mat3 M_p(pLocalLeft, pLocalUp, pLocalForward);
      mat3 M(WorldLeft, WorldUp, WorldForward);
      return glm::inverse(M_p) * M * (globalPos - GetParentPosition());
    }
    // localposition relative to its parent to global position
    const vec3 LocalToGlobal(vec3 localPos) {
      vec3 pLocalForward, pLocalLeft, pLocalUp;
      GetParentLocalAxis(pLocalForward, pLocalLeft, pLocalUp);
      mat3 M_p(pLocalLeft, pLocalUp, pLocalForward);
      mat3 M(WorldLeft, WorldUp, WorldForward);
      return (glm::inverse(M) * M_p * localPos) + GetParentPosition();
    }

    const vec3 GetParentScale() {
      if (parent == nullptr) return vec3(1.0f);
      else return parent->m_scale;
    }

    const vec3 GetParentPosition() {
      if (parent == nullptr) return vec3(0.0f);
      else return parent->m_position;
    }

    // (self.orien = parent.orien * self.localRot)
    // or (self.globalRot = parent.globalRot * self.localRot)
    const quat GetParentOrientation() {
      Entity *current = parent;
      quat q(1.0f, vec3(0.0f)); // root.parent.orien
      stack<quat> s;
      while (current != nullptr) {
        s.push(current->localRotation); // cur.localRot
        current = current->parent;
      }
      while (!s.empty()) {
        q = q * s.top();
        s.pop();
      }
      return q;
    }

    void GetParentLocalAxis(vec3 &pLocalForward, vec3 &pLocalLeft, vec3 &pLocalUp) {
      if (parent == nullptr) {
        pLocalForward = WorldForward;
        pLocalLeft = WorldLeft;
        pLocalUp = WorldUp;
      } else {
        parent->UpdateLocalAxis();
        pLocalForward = parent->LocalForward;
        pLocalLeft = parent->LocalLeft;
        pLocalUp = parent->LocalUp;
      }
    }

    template <typename T, typename... Args> void AddComponent(Args &&...args) {
      MGR->AddComponent<T>(ID, std::forward<Args>(args)...);
    }

    template <typename T> void RemoveComponent() {
      MGR->RemoveComponent<T>(ID);
    }

    template <typename T> const bool HasComponent() {
      return MGR->HasComponent<T>(ID);
    }

    template <typename T> T &GetComponent() { return MGR->GetComponent<T>(ID); }

    void Destroy() { MGR->DestroyEntity(ID); }

    void AssignChild(Entity *c) {
      if (c == nullptr)
        throw std::runtime_error("can't set null pointer as child");
      if (std::find(children.begin(), children.end(), c) != children.end()) {
        printf("entity %d is already the child of entity %d", (unsigned int)c->ID, (unsigned int)ID);
        return;
      }
      if (c->parent != nullptr) {
        // remove c from its parent's list
        auto it = std::find(c->parent->children.begin(), c->parent->children.end(), c);
        if (it == c->parent->children.end())
          throw std::runtime_error("entity is not a child of its parent");
        c->parent->children.erase(it);
      }
      children.push_back(c);
      c->parent = this;
      // update the local properties with global properties
      c->SetGlobalPosition(c->Position());
      c->SetGlobalRotation(c->Rotation());
      c->SetGlobalScale(c->Scale());
    }

    mat4 GetModelMatrix() {
      return glm::translate(mat4(1.0f), m_position) * glm::mat4_cast(Rotation()) * glm::scale(mat4(1.0f), m_scale);
      // return glm::translate(mat4(1.0f), m_position) * glm::transpose(glm::mat4_cast(Rotation())) * glm::scale(mat4(1.0f), m_scale);
    }

    EntityID ID;
    string name = "New Entity ";
    Entity *parent = nullptr;
    vector<Entity *> children;

  protected:
    EntityManager *MGR;

    vec3 m_position;
    vec3 m_scale;
    vec3 m_eulerAngles;

  };

  EntityManager() : entityCount(0) {
    nullEntity = new Entity((EntityID)(-1), this);
    for (EntityID entity = 0u; entity < MAX_ENTITY_COUNT; ++entity) {
      availableEntities.push(entity);
    }
    HierarchyRoots.reserve(MAX_ENTITY_COUNT);
  }
  EntityManager(const EntityManager &) = delete;
  const EntityManager &operator=(EntityManager &) = delete;
  ~EntityManager() { delete nullEntity; }

  static EntityManager &Ref() {
    static EntityManager reference;
    return reference;
  }

  // Call the start function of all the systems
  void Start() {
    for (auto &system : registeredSystems) {
      system.second->Start();
    }
  }

  // Update all the registered systems
  void Update() {
    // update the transforms first
    recomputeLocalAxis();
    rebuildHierarchyStructure();

    // other systems
    for (auto &system : registeredSystems) {
      system.second->Update();
    }
  }

  // Destroy all the registered systems
  void Destroy() {
    for (auto &system : registeredSystems) {
      system.second->Destroy();
    }
  }

  Entity *AddNewEntity() {
    const EntityID id = addNewEntity();
    entities.insert(
        std::make_pair(id, std::move(std::make_shared<Entity>(id, this))));
    entities[id]->name += std::to_string(id);
    return &(*(entities[id]));
  }

  Entity *EntityFromID(const EntityID entity) {
    if (entity >= MAX_ENTITY_COUNT)
      throw std::runtime_error(
          "EntityID out of range (MAX_ENTITY_COUNT) during EntityFromID");
    if (entities.count(entity) == 0) {
      Console.Log("[error]: No entity match this id: %ld\n", entity);
      return nullEntity;
    } else {
      return &(*(entities.at(entity)));
    }
  }

  vector<Entity *> GetActiveEntities() {
    vector<Entity *> result;
    for (auto &entity : entities) {
      result.push_back(&(*entity.second));
    }
    return result;
  }

  // if one entity with children is destroyed, all its children entities will
  // also be destroyed
  void DestroyEntity(const EntityID entity) {
    if (entity >= MAX_ENTITY_COUNT)
      throw std::runtime_error("Destroying entity out of range");
    if (entitiesSignatures.find(entity) == entitiesSignatures.end())
      throw std::runtime_error("Destroying entity do not exists");
    auto children = entities[entity]->children;
    entitiesSignatures.erase(entity);
    entities.erase(entity);
    for (auto &array : componentsArrays) {
      array.second->Erase(entity);
    }
    // the destroyed entity won't appear in any systems
    for (auto &system : registeredSystems) {
      system.second->RemoveEntity(entity);
    }
    entityCount--;
    availableEntities.push(entity);
    for (auto child : children)
      DestroyEntity(child->ID);
  }

  template <typename T, typename... Args>
  void AddComponent(const EntityID entity, Args &&...args) {
    if (entity >= MAX_ENTITY_COUNT)
      throw std::runtime_error(
          "EntityID out of range (MAX_ENTITY_COUNT) during AddComponent");
    if (entitiesSignatures[entity].get()->size() >= MAX_COMPONENT_COUNT)
      throw std::runtime_error(
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
    if (entity >= MAX_ENTITY_COUNT)
      throw std::runtime_error(
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
    if (entity >= MAX_ENTITY_COUNT)
      throw std::runtime_error(
          "EntityID out of range (MAX_ENTITY_COUNT) during GetComponent");
    const ComponentTypeID compType = ComponentType<T>();
    return GetComponentList<T>()->Get(entity);
  }

  // iterate through the signature of the entity to find the component with
  // indicated type
  template <typename T> const bool HasComponent(const EntityID entity) {
    if (entity >= MAX_ENTITY_COUNT)
      throw std::runtime_error(
          "EntityID out of range (MAX_ENTITY_COUNT) during HasComponent");
    const EntitySignature signature = *(entitiesSignatures.at(entity));
    const ComponentTypeID compType = ComponentType<T>();
    auto it = std::find(signature.begin(), signature.end(), compType);
    return it != signature.end();
  }

  // register system at runtime
  template <typename T> void RegisterSystem() {
    const SystemTypeID systemType = SystemType<T>();
    if (registeredSystems.count(systemType) != 0)
      throw std::runtime_error("System already registered");
    auto system = std::make_shared<T>();
    // add entities that might belongs to the system
    for (EntityID entity = 0; entity < entityCount; ++entityCount) {
      AddEntityToSystem(entity, system.get());
    }
    // don't start the system during registration
    // system->Start();
    registeredSystems[systemType] = std::move(system);
  }

  template <typename T> void UnRegisterSystem() {
    const SystemTypeID systemType = SystemType<T>();
    if (registeredSystems.count(systemType) == 0)
      throw std::runtime_error("System not registered");
    registeredSystems.erase(systemType);
  }

  template <typename T> T *GetSystemInstance() {
    const SystemTypeID systemType = SystemType<T>();
    if (registeredSystems.count(systemType) == 0)
      throw std::runtime_error("System not registered");
    return (T *)(registeredSystems[systemType].get());
  }

  vector<Entity *> HierarchyRoots;

private:
  // create a component list that stores a specified type of components
  template <typename T> void AddComponentList() {
    const ComponentTypeID compType = ComponentType<T>();
    if (componentsArrays.find(compType) != componentsArrays.end())
      throw std::runtime_error("Component list already registered");
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
    if (entitiesSignatures.find(entity) != entitiesSignatures.end())
      throw std::runtime_error("Signature not found");
    entitiesSignatures[entity] = std::move(std::make_shared<EntitySignature>());
  }

  std::shared_ptr<EntitySignature> GetEntitySignature(const EntityID entity) {
    // assert will be triggered when the condition is false
    if (entitiesSignatures.find(entity) == entitiesSignatures.end())
      throw std::runtime_error("Signature not found");
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
    for (const auto compType : systemSignature) {
      if (GetEntitySignature(entity)->count(compType) == 0) {
        return false;
      }
    }
    return true;
  }

  const EntityID addNewEntity() {
    const EntityID id = availableEntities.front();
    AddEntitySignature(id); // create a signature for the entity
    availableEntities.pop();
    entityCount++;
    return id; // this entity can now access some components
  }

  // the first
  void recomputeLocalAxis() {
    for (auto entity : entities) {
      entity.second->UpdateLocalAxis();
    }
  }

  // the second
  void rebuildHierarchyStructure() {
    HierarchyRoots.clear();
    queue<Entity*> q;
    for (auto entity : entities) {
      if (entity.second->parent == nullptr) {
        q.push(entity.second.get());
        HierarchyRoots.push_back(entity.second.get());
      }
    }
    while (!q.empty()) {
      auto ent = q.front();
      q.pop();
      for (auto child : ent->children) {
        // update global positions with local positions
        child->m_position = child->LocalToGlobal(child->localPosition);
        child->m_eulerAngles = glm::eulerAngles(child->GetParentOrientation() * child->localRotation);
        child->m_scale = child->parent->m_scale * child->localScale;
        q.push(child);
      }
    }
  }

  // how many entities have been created
  EntityID entityCount;
  Entity *nullEntity;
  std::queue<EntityID> availableEntities;
  // every time a entity is created, a signature for it will also be created
  std::map<EntityID, std::shared_ptr<Entity>> entities;
  std::map<EntityID, std::shared_ptr<EntitySignature>> entitiesSignatures;
  std::map<SystemTypeID, std::shared_ptr<BaseSystem>> registeredSystems;
  std::map<ComponentTypeID, std::shared_ptr<IComponentList>> componentsArrays;
};

static EntityManager &EManager = EntityManager::Ref();

}; // namespace ECS

using Entity = ECS::EntityManager::Entity;
// alias to entity
using Transform = ECS::EntityManager::Entity;