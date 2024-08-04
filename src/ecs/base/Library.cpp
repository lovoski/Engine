#include "EntityManager.hpp"

// include all the components for manual serialization registration
#include "ecs/components/Camera.hpp"
#include "ecs/components/Lights.hpp"
#include "ecs/components/Material.hpp"
#include "ecs/components/MeshRenderer.hpp"

// the gui should be rendered after the main render
#include "ecs/systems/gui/GUISystem.hpp"
#include "ecs/systems/camera/CameraSystem.hpp"
#include "ecs/systems/light/LightSystem.hpp"
#include "ecs/systems/render/RenderSystem.hpp"

vec3 Entity::WorldUp = vec3(0.0f, 1.0f, 0.0f);
vec3 Entity::WorldLeft = vec3(1.0f, 0.0f, 0.0f);
vec3 Entity::WorldForward = vec3(0.0f, 0.0f, 1.0f);

namespace ECS {

void EntityManager::Start() {
  // register all the systems
  RegisterSystem<RenderSystem>();
  RegisterSystem<GuiSystem>();
  RegisterSystem<LightSystem>();
  RegisterSystem<CameraSystem>();

  // start all the systems
  GetSystemInstance<RenderSystem>()->Start();
  // start gui system after the render system
  GetSystemInstance<GuiSystem>()->Start();
  GetSystemInstance<LightSystem>()->Start();
  GetSystemInstance<CameraSystem>()->Start();
}

void EntityManager::Update() {
  // update the transforms first
  recomputeLocalAxis();
  rebuildHierarchyStructure();

  for (auto &system : registeredSystems) {
    system.second->Update();
  }

  // do the rendering
  GetSystemInstance<RenderSystem>()->BeginRender();
  GetSystemInstance<GuiSystem>()->Render();
  GetSystemInstance<RenderSystem>()->EndRender();
}

void EntityManager::Destroy() {
  // destroy gui system before render system
  GetSystemInstance<GuiSystem>()->Destroy();
  GetSystemInstance<RenderSystem>()->Destroy();
  GetSystemInstance<LightSystem>()->Destroy();
  GetSystemInstance<CameraSystem>()->Destroy();
}

Entity *EntityManager::AddNewEntity() {
  const EntityID id = addNewEntity();
  entities.insert(
      std::make_pair(id, std::move(std::make_shared<Entity>(id, this))));
  entities[id]->name += std::to_string(id);
  return &(*(entities[id]));
}

Entity *EntityManager::EntityFromID(const EntityID entity) {
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

vector<Entity *> EntityManager::GetActiveEntities() {
  vector<Entity *> result;
  for (auto &entity : entities) {
    result.push_back(&(*entity.second));
  }
  return result;
}

void EntityManager::DestroyEntity(const EntityID entity) {
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

};
