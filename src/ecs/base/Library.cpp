#include "EntityManager.hpp"

// include all the components for manual serialization registration
#include "ecs/components/Camera.hpp"
#include "ecs/components/Lights.hpp"
#include "ecs/components/Material.hpp"
#include "ecs/components/MeshRenderer.hpp"

vec3 Entity::WorldUp = vec3(0.0f, 1.0f, 0.0f);
vec3 Entity::WorldLeft = vec3(1.0f, 0.0f, 0.0f);
vec3 Entity::WorldForward = vec3(0.0f, 0.0f, 1.0f);

// stores current states in a scene file
Json ECS::EntityManager::CaptureStatesAsScene() {
  Json json;
  // the entity data
  // for (auto entity : entities) {
  //   string currentEntityID = std::to_string((unsigned int)entity.first);
  //   Reflection::Serialize(json["entities"][currentEntityID], *(entity.second));
  //   json["entities"][currentEntityID]["parent"] =
  //       entity.second->parent == nullptr
  //           ? -1
  //           : (unsigned int)(entity.second->parent->ID);
  //   vector<unsigned int> componentSignature;
  //   for (auto ent : *(entitiesSignatures[entity.first]))
  //     componentSignature.push_back((unsigned int)ent);
  //   json["entities"][currentEntityID]["componentSignature"] =
  //       componentSignature;
  // }
  // // if more components created, they need to be registered here to be serialized
  // for (auto camera : GetComponentList<Camera>()->data) {
  //   Reflection::Serialize(
  //       json["components"]["camera"][std::to_string(camera.GetID())], camera);
  // }
  // for (auto light : GetComponentList<BaseLight>()->data) {
  //   Reflection::Serialize(
  //       json["components"]["light"][std::to_string(light.GetID())], light);
  // }
  // for (auto material : GetComponentList<BaseMaterial>()->data) {
  //   Reflection::Serialize(
  //       json["components"]["material"][std::to_string(material.GetID())],
  //       material);
  // }
  // for (auto renderer : GetComponentList<MeshRenderer>()->data) {
  //   Reflection::Serialize(
  //       json["components"]["renderer"][std::to_string(renderer.GetID())],
  //       renderer);
  // }
  return json;
}

// restore states from a json object
void ECS::EntityManager::InitializeFromScene(Json json) {
  // // destroy current scene (entities, components)
  // GetComponentList<Camera>()->data.clear();
  // GetComponentList<BaseLight>()->data.clear();
  // GetComponentList<BaseMaterial>()->data.clear();
  // GetComponentList<MeshRenderer>()->data.clear();

  // for (auto root : HierarchyRoots)
  //   DestroyEntity(root->ID);
  // HierarchyRoots.clear();
  // // reload scene from the file
  
}