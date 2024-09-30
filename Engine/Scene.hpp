/**
 * Each scene instance will maintain its own instances
 * of the entities, components and systems. The `Update`
 * function in each scene does the logic update (update hierarchy
 * transform, system related to positions etc.), the `RenderBegin`
 * function render the scene from activeCamera to the frameBuffer.
 * The `RenderEnd` swaps the framebuffer.
 *
 * However, the scene acts as a world query handle, so it being a singleton
 * could really help with the coding.
 */

#pragma once

#include "Base/BaseComponent.hpp"
#include "Base/BaseSystem.hpp"
#include "Base/ComponentList.hpp"
#include "Base/Scriptable.hpp"
#include "Base/Types.hpp"

#include "Global.hpp"

namespace aEngine {

class Engine;
class Entity;

struct SceneContext {
  // Keep a reference to the window
  GLFWwindow *window;
  Engine *engine;

  // Camera related
  bool hasActiveCamera;
  EntityID activeCamera;

  // Debug draw options
  bool enableDebugDraw;

  // Scene serialization
  std::string sceneFilePath;
  glm::vec2 sceneWindowSize;
  glm::vec2 sceneWindowPos;

  // Other settings
  glm::vec2 currentMousePosition;

  // Time related
  float lastTime;
  float deltaTime;
  float renderTime;
  float updateTime;
  float debugDrawTime;
  float hierarchyUpdateTime;

  // Update deltaTime
  void Tick() {
    float c = glfwGetTime();
    deltaTime = c - lastTime;
    lastTime = c;
  }

  void Reset() {
    hasActiveCamera = false;
    activeCamera = (EntityID)(-1);
    enableDebugDraw = true;
    // Don't change the size and position of scene window
    sceneFilePath = "::defaultScene";

    lastTime = 0.0f;
    deltaTime = 0.0f;
    renderTime = 0.0f;
    updateTime = 0.0f;
    debugDrawTime = 0.0f;
    hierarchyUpdateTime = 0.0f;
  }

  template <typename Archive>
  void serialize(Archive &archive, const unsigned int version) {
    archive &hasActiveCamera;
    archive &activeCamera;
    archive &enableDebugDraw;
    archive &sceneFilePath;
    archive &lastTime;
    archive &deltaTime;
    archive &renderTime;
    archive &updateTime;
    archive &debugDrawTime;
    archive &hierarchyUpdateTime;
  }
};

class Scene {
public:
  Scene();
  Scene(const Scene &) = delete;
  const Scene &operator=(Scene &) = delete;
  ~Scene();

  static Scene &Ref() {
    static Scene reference;
    return reference;
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
    std::shared_ptr<T> component =
        std::make_shared<T>(entity, std::forward<Args>(args)...);
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

  // find the component belongs to some entity, returns nullptr when the
  // entity don't have this component.
  template <typename T> std::shared_ptr<T> GetComponent(const EntityID entity) {
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

  // Get all the entities in this scene, it's recommended to use the
  // `GetSystemInstance` function to get the instance of system maintaining a
  // specific type of entity for easier query.
  const std::map<EntityID, std::shared_ptr<Entity>> GetEntities() const {
    return entities;
  }

  // Check if one entity is valid,
  // return `false` and set id to -1 if not.
  bool EntityValid(EntityID &id);

  bool LoopCursorInSceneWindow();

  bool InSceneWindow(float x, float y) {
    return x >= Context.sceneWindowPos.x &&
           x <= Context.sceneWindowPos.x + Context.sceneWindowSize.x &&
           y >= Context.sceneWindowPos.y &&
           y <= Context.sceneWindowPos.y + Context.sceneWindowSize.y;
  }

  std::vector<Entity *> HierarchyRoots;

  SceneContext Context;

  // Call the start function of all the systems,
  // initialize scene context
  void Start();

  // The main loop, update all the registered systems, do the main rendering
  void Update();

  // Do the main rendering, this function is called internally in Update,
  // only call this function when there's additional need to do it
  void ForceRender();

  // Finish the render, swap framebuffer to display
  void RenderEnd();

  // Destroy all the registered systems
  void Destroy();

  // Reset the scene context
  void Reset();

  // Configure the default scene with a default camera
  void SetupDefaultScene();

  void PlotSceneProfile();

  float GetTime() { return glfwGetTime(); }

  // Serialize current scene into a binary file,
  // returns true for success.
  bool Save(std::string path);
  // Reset the scene from a binary file
  // returns true for success.
  bool Load(std::string path);

  std::shared_ptr<Entity> AddNewEntity();
  std::shared_ptr<Entity> EntityFromID(const EntityID entity);

  // if one entity with children is destroyed, all its children entities will
  // also be destroyed
  void DestroyEntity(const EntityID entity);

  const std::map<ComponentTypeID, std::shared_ptr<IComponentList>> &
  GetAllComponentArrays() const {
    return componentsArrays;
  }

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
    if (BelongToSystem(entity, system->GetSignature(),
                       system->GetSignatureOne())) {
      system->AddEntity(entity);
    } else {
      system->RemoveEntity(entity);
    }
  }

  // check if an entity belongs to the system by comparing their signatures
  // the entity should have all the components described with
  // `AddComponentSignatureRequireAll` and at least one component described with
  // `AddComponentSignatureRequireOne`.
  bool BelongToSystem(const EntityID entity,
                      const EntitySignature &systemSignature,
                      const EntitySignature &systemSignatureOne);

  const EntityID addNewEntity() {
    const EntityID id = availableEntities.front();
    AddEntitySignature(id); // create a signature for the entity
    availableEntities.pop();
    entityCount++;
    return id; // this entity can now access some components
  }

  // the first
  void recomputeLocalAxis();

  // The HierarchyRoot variable will get clear and updated by this function
  void rebuildHierarchyStructure();

  // how many entities have been created
  EntityID entityCount;
  std::queue<EntityID> availableEntities;
  // every time a entity is created, a signature for it will also be created
  std::map<EntityID, std::shared_ptr<Entity>> entities;
  std::map<EntityID, std::shared_ptr<EntitySignature>> entitiesSignatures;
  std::map<SystemTypeID, std::shared_ptr<BaseSystem>> registeredSystems;
  std::map<ComponentTypeID, std::shared_ptr<IComponentList>> componentsArrays;
};

static Scene &GWORLD = Scene::Ref();

}; // namespace aEngine
