#include "EntityManager.hpp"

// include all the components for manual serialization registration
#include "ecs/components/Camera.hpp"
#include "ecs/components/Lights.hpp"
#include "ecs/components/Material.hpp"
#include "ecs/components/MeshRenderer.hpp"

#include "engine/EditorWindows.hpp"

#include "ecs/systems/camera/CameraSystem.hpp"
#include "ecs/systems/light/LightSystem.hpp"
#include "ecs/systems/render/RenderSystem.hpp"

vec3 Entity::WorldUp = vec3(0.0f, 1.0f, 0.0f);
vec3 Entity::WorldLeft = vec3(1.0f, 0.0f, 0.0f);
vec3 Entity::WorldForward = vec3(0.0f, 0.0f, 1.0f);

// stores current states in a scene file
Json ECS::EntityManager::CaptureStatesAsScene() {
  Json json;
  // the entity data
  for (auto entity : entities) {
    string currentEntityID = std::to_string((unsigned int)entity.first);
    entity.second->Serialize(json["entities"][currentEntityID]);
  }
  // if more components created, they need to be registered here to be serialized
  for (auto camera : GetComponentList<Camera>()->data) {
    camera.Serialize(
        json["components"]["camera"][std::to_string(camera.GetID())]);
  }
  for (auto material : GetComponentList<BaseMaterial>()->data) {
    material.Serialize(json["components"]["material"][std::to_string(material.GetID())]);
  }
  for (auto light : GetComponentList<BaseLight>()->data) {
    light.Serialize(
        json["components"]["light"][std::to_string(light.GetID())]);
  }
  for (auto renderer : GetComponentList<MeshRenderer>()->data) {
    renderer.Serialize(
        json["components"]["renderer"][std::to_string(renderer.GetID())]);
  }
  // scene specific settings
  EntityID activeCamera;
  EditorContext.GetActiveCamera(activeCamera);
  json["scene"]["activeCamera"] = (int)activeCamera;
  return json;
}

// restore states from a json object
void ECS::EntityManager::InitializeFromScene(Json &json) {
  // destroy current scene (entities, components)
  for (auto root : HierarchyRoots)
    DestroyEntity(root->ID);
  GetComponentList<Camera>()->data.clear();
  GetComponentList<BaseLight>()->data.clear();
  GetComponentList<BaseMaterial>()->data.clear();
  GetComponentList<MeshRenderer>()->data.clear();
  HierarchyRoots.clear();
  entitiesSignatures.clear();
  // reset editor context
  EditorContext.Reset();

  // reload scene from the file
  std::map<EntityID, EntityID> old2new;
  for (auto entJson : json["entities"].items()) {
    EntityID old = (EntityID)std::stoi(entJson.key());
    auto newEntity = EManager.AddNewEntity();
    newEntity->name = entJson.value()["name"];
    old2new[old] = newEntity->ID;
  }
  for (auto entJson : json["entities"].items()) {
    // cout << entJson << endl;
    EntityID old = (EntityID)std::stoi(entJson.key());
    EntityID newID = old2new[old];
    auto newEntity = EManager.EntityFromID(newID);
    newEntity->SetGlobalPosition(entJson.value()["p"].get<vec3>());
    newEntity->SetGlobalScale(entJson.value()["s"].get<vec3>());
    newEntity->SetGlobalRotation(entJson.value()["r"].get<vec3>());
    if (entJson.value().value("parent", "none") != "none") {
      auto oldParentID = (EntityID)std::stoi(entJson.value().value("parent", "none"));
      auto parent = EManager.EntityFromID(old2new[oldParentID]);
      parent->AssignChild(newEntity);
    }
    // reload the components
    for (auto compJson : json["components"]["camera"].items()) {
      EntityID belongs = (EntityID)std::stoi(compJson.key());
      if (belongs == old) {
        // this component belongs to the entity
        newEntity->AddComponent<Camera>();
        newEntity->GetComponent<Camera>().Deserialize(json["components"]["camera"][std::to_string((int)old)]);
      }
    }
    for (auto compJson : json["components"]["material"].items()) {
      EntityID belongs = (EntityID)std::stoi(compJson.key());
      if (belongs == old) {
        // this component belongs to the entity
        newEntity->AddComponent<BaseMaterial>();
        newEntity->GetComponent<BaseMaterial>().Deserialize(json["components"]["material"][std::to_string((int)old)]);
      }
    }
    for (auto compJson : json["components"]["light"].items()) {
      EntityID belongs = (EntityID)std::stoi(compJson.key());
      if (belongs == old) {
        // this component belongs to the entity
        newEntity->AddComponent<BaseLight>();
        newEntity->GetComponent<BaseLight>().Deserialize(json["components"]["light"][std::to_string((int)old)]);
      }
    }
    for (auto compJson : json["components"]["renderer"].items()) {
      EntityID belongs = (EntityID)std::stoi(compJson.key());
      if (belongs == old) {
        // this component belongs to the entity
        newEntity->AddComponent<MeshRenderer>();
        newEntity->GetComponent<MeshRenderer>().Deserialize(json["components"]["renderer"][std::to_string((int)old)]);
      }
    }
  }
  // reset the scene variables
  EditorContext.SetActiveCamera(old2new[json["scene"]["activeCamera"]]);
}