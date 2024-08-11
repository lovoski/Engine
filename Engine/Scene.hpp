/**
 * Each scene instance will maintain its own instances
 * of the entities, components and systems. The `Update` 
 * function in each scene does the logic update (update hierarchy 
 * transform, system related to positions etc.), the `RenderBegin` 
 * function render the scene from activeCamera to the frameBuffer. 
 * The `RenderEnd` swaps the framebuffer.
 */

#pragma once

#include "Base/Types.hpp"
#include "Base/BaseSystem.hpp"
#include "Base/ComponentList.hpp"
#include "Base/BaseComponent.hpp"

#include "Global.hpp"
#include "Component/Light.hpp"
#include "System/Render/FrameBuffer.hpp"

namespace aEngine {

struct SceneContext {
  FrameBuffer *frameBuffer;
  
  // Keep a reference to the window
  GLFWwindow *window;

  // Grid options
  bool showGrid;
  unsigned int gridSize;
  glm::vec3 gridColor;

  // Camera related
  bool hasActiveCamera;
  EntityID activeCamera;

  // Debug draw options
  bool enableDebugDraw;

  // Scene serialization
  std::string sceneFilePath;
  glm::vec2 sceneWindowSize;
  glm::vec2 sceneWindowPos;

  // Scene lights
  std::vector<Light> activeLights;

  void Reset() {
    showGrid = true;
    gridSize = 10;
    gridColor = glm::vec3(1.0f);
    hasActiveCamera = false;
    activeCamera = (EntityID)(-1);
    enableDebugDraw = true;
    // Don't change the size and position of scene window
    sceneFilePath = "::defaultScene";
    activeLights.clear();
  }
};

class Scene {
public:
  class Entity {
  public:
    friend class Scene;

    Entity(EntityID id, Scene *scene) : ID(id), sceneManager(scene) {
      m_scale = glm::vec3(1.0f);
      m_position = glm::vec3(0.0f);
      m_eulerAngles = glm::vec3(0.0f);
    }
    ~Entity() {
      if (parent != nullptr) {
        // remove this child from its parent's child list
        if (parent->children.size() != 0) {
          auto it =
              std::find(parent->children.begin(), parent->children.end(), this);
          if (it != parent->children.end())
            parent->children.erase(it);
        }
        parent = nullptr;
      }
      children.clear();
    }

    // global scale
    const glm::vec3 Scale() { return m_scale; };
    // global rotation (pitch, yaw, roll) = (x, y, z)
    // in radians
    const glm::vec3 EulerAngles() { return m_eulerAngles; }
    // euler angles in degree
    const glm::vec3 EulerAnglesDegree() { return glm::degrees(m_eulerAngles); }
    // global rotation
    const glm::quat Rotation() { return glm::quat(m_eulerAngles); }
    // position under world axis
    const glm::vec3 Position() { return m_position; }

    // scale relative to its parent's axis
    glm::vec3 localScale = glm::vec3(1.0f);
    // rotation relative to its parent's axis
    glm::quat localRotation = glm::quat(1.0f, glm::vec3(0.0f));
    // position relative to its parent's axis
    glm::vec3 localPosition = glm::vec3(0.0f);

    static glm::vec3 WorldUp, WorldLeft, WorldForward;

    // local axis are updated at the start of each loop
    glm::vec3 LocalUp, LocalLeft, LocalForward;

    // set global position
    void SetGlobalPosition(glm::vec3 p) {
      // change global position, modify local position to satisfy the global
      // position
      m_position = p;
      // the local axis will be updated in GlobalToLocal function call
      localPosition = GlobalToLocal(p);
    }
    // set global rotation (pitch, yaw, roll) = (x, y, z)
    // in degrees
    void SetGlobalRotationDegree(glm::vec3 a) {
      a = glm::radians(a);
      SetGlobalRotation(a);
    }
    // set global rotation (pitch, yaw, roll) = (x, y, z)
    // in radians
    void SetGlobalRotation(glm::vec3 a) {
      glm::quat parentOrien = GetParentOrientation();
      localRotation = glm::inverse(parentOrien) * glm::quat(a);
      m_eulerAngles = a;
      UpdateLocalAxis();
    }
    // set global rotation
    void SetGlobalRotation(glm::quat q) {
      glm::quat parentOrien = GetParentOrientation();
      localRotation = glm::inverse(parentOrien) * q;
      m_eulerAngles = glm::eulerAngles(q);
      UpdateLocalAxis();
    }
    // set global scale
    void SetGlobalScale(glm::vec3 s) {
      localScale = s / GetParentScale();
      m_scale = s;
    }

    void UpdateLocalAxis() {
      glm::quat q = GetParentOrientation() * localRotation;
      LocalForward = q * WorldForward;
      LocalLeft = q * WorldLeft;
      LocalUp = q * WorldUp;
    }

    // global position to the local position relative to its parent
    const glm::vec3 GlobalToLocal(glm::vec3 globalPos) {
      glm::vec3 pLocalForward, pLocalLeft, pLocalUp;
      GetParentLocalAxis(pLocalForward, pLocalLeft, pLocalUp);
      // TODO: always remembers how to intiailize a matrix
      glm::mat3 M_p(pLocalLeft, pLocalUp, pLocalForward);
      glm::mat3 M(WorldLeft, WorldUp, WorldForward);
      return glm::inverse(M_p) * M * (globalPos - GetParentPosition());
    }
    // localposition relative to its parent to global position
    const glm::vec3 LocalToGlobal(glm::vec3 localPos) {
      glm::vec3 pLocalForward, pLocalLeft, pLocalUp;
      GetParentLocalAxis(pLocalForward, pLocalLeft, pLocalUp);
      glm::mat3 M_p(pLocalLeft, pLocalUp, pLocalForward);
      glm::mat3 M(WorldLeft, WorldUp, WorldForward);
      return (glm::inverse(M) * M_p * localPos) + GetParentPosition();
    }

    const glm::vec3 GetParentScale() {
      if (parent == nullptr)
        return glm::vec3(1.0f);
      else
        return parent->m_scale;
    }

    const glm::vec3 GetParentPosition() {
      if (parent == nullptr)
        return glm::vec3(0.0f);
      else
        return parent->m_position;
    }

    // (self.orien = parent.orien * self.localRot)
    // or (self.globalRot = parent.globalRot * self.localRot)
    const glm::quat GetParentOrientation() {
      Entity *current = parent;
      glm::quat q(1.0f, glm::vec3(0.0f)); // root.parent.orien
      std::stack<glm::quat> s;
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

    void GetParentLocalAxis(glm::vec3 &pLocalForward, glm::vec3 &pLocalLeft,
                            glm::vec3 &pLocalUp) {
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

    void Serialize(Json &json) {
      json["p"] = m_position;
      json["r"] = m_eulerAngles;
      json["s"] = m_scale;
      json["parent"] = parent == nullptr ? "none" : std::to_string((int)parent->ID);
      json["name"] = name;
    }

    template <typename T, typename... Args> void AddComponent(Args &&...args) {
      sceneManager->AddComponent<T>(ID, std::forward<Args>(args)...);
    }

    template <typename T> void RemoveComponent() {
      sceneManager->RemoveComponent<T>(ID);
    }

    template <typename T> const bool HasComponent() {
      return sceneManager->HasComponent<T>(ID);
    }

    template <typename T> T &GetComponent() { return sceneManager->GetComponent<T>(ID); }

    void Destroy() { sceneManager->DestroyEntity(ID); }

    void AssignChild(Entity *c) {
      if (c == nullptr)
        throw std::runtime_error("can't set null pointer as child");
      // make sure c is not a ancestor of current entity
      if (parent != nullptr) {
        auto current = parent;
        while (current != nullptr) {
          if (current == c) {
            printf("can't set directly revert ancestor child relation\n");
            return;
          }
          current = current->parent;
        }
      }
      if (std::find(children.begin(), children.end(), c) != children.end()) {
        printf("entity %d is already the child of entity %d\n",
               (unsigned int)c->ID, (unsigned int)ID);
        return;
      }
      if (c->parent != nullptr) {
        // remove c from its parent's list
        auto it = std::find(c->parent->children.begin(),
                            c->parent->children.end(), c);
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

    glm::mat4 GetModelMatrix() {
      return glm::translate(glm::mat4(1.0f), m_position) *
             glm::mat4_cast(Rotation()) * glm::scale(glm::mat4(1.0f), m_scale);
      // return glm::translate(mat4(1.0f), m_position) *
      // glm::transpose(glm::mat4_cast(Rotation())) * glm::scale(mat4(1.0f),
      // m_scale);
    }

    EntityID ID;
    std::string name = "New Entity ";
    Entity *parent = nullptr;
    std::vector<Entity *> children;

  protected:
    Scene *sceneManager;

    glm::vec3 m_position;
    glm::vec3 m_scale;
    glm::vec3 m_eulerAngles;
  };

  Scene(float sceneWidth, float sceneHeight);
  Scene(const Scene &) = delete;
  const Scene &operator=(Scene &) = delete;
  ~Scene();

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
    system->sceneManager = this;
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

  // Set the parameter `camera` to active camera and returns true
  // if there's an active, return false otherwise.
  bool GetActiveCamera(EntityID &camera);

  // Set active camera to the parameter and return true if `camera`
  // is a valid camera entity (has camera component), otherwise returns false.
  bool SetActiveCamera(EntityID camera);

  // Serialize current scene to a json file
  Json Serialize();

  // Reset current scene from a json file
  void DeserializeReset(Json &json);

  std::vector<Entity *> HierarchyRoots;

  SceneContext Context;

  // Call the start function of all the systems,
  // initialize scene context
  void Start();

  // Update all the registered systems
  void Update();

  void RenderBegin();
  void RenderEnd();

  // Destroy all the registered systems
  void Destroy();

  // Reset the scene context
  void Reset();

  Entity *AddNewEntity();
  Entity *EntityFromID(const EntityID entity);

  // if one entity with children is destroyed, all its children entities will
  // also be destroyed
  void DestroyEntity(const EntityID entity);

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
    std::queue<Entity *> q;
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
        child->m_eulerAngles = glm::eulerAngles(child->GetParentOrientation() *
                                                child->localRotation);
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

using Entity = Scene::Entity;
// alias to entity
using Transform = Scene::Entity;

};
