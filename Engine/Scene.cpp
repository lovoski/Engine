#include "Scene.hpp"

#include "Component/Camera.hpp"
#include "Component/Light.hpp"
#include "Component/MeshRenderer.hpp"
#include "Component/NativeScript.hpp"

#include "System/Animation/AnimationSystem.hpp"
#include "System/NativeScript/NativeScriptSystem.hpp"
#include "System/Render/FrameBuffer.hpp"
#include "System/Render/LightSystem.hpp"
#include "System/Render/RenderSystem.hpp"

namespace aEngine {

Scene::Scene() {
  // create the entities
  entityCount = 0;
  for (EntityID entity = 0u; entity < MAX_ENTITY_COUNT; ++entity) {
    availableEntities.push(entity);
  }
  HierarchyRoots.reserve(MAX_ENTITY_COUNT);

  // create the context with default size
  Context.Reset();
}

Scene::~Scene() {}

void Scene::Start() {
  // register all the systems
  RegisterSystem<RenderSystem>();
  RegisterSystem<LightSystem>();
  RegisterSystem<AnimationSystem>();
  RegisterSystem<NativeScriptSystem>();

  // start all the systems
  GetSystemInstance<RenderSystem>()->Start();
  GetSystemInstance<LightSystem>()->Start();
  GetSystemInstance<AnimationSystem>()->Start();
  GetSystemInstance<NativeScriptSystem>()->Start();
}

void Scene::Update() {
  // tick the timer
  Context.Tick();
  float t0 = GetTime();
  // update the transforms first
  recomputeLocalAxis();
  rebuildHierarchyStructure();
  float t1 = GetTime();

  // call update
  for (auto &system : registeredSystems)
    system.second->Update(Context.deltaTime);

  // call late update
  GetSystemInstance<NativeScriptSystem>()->LateUpdate(Context.deltaTime);
  float t2 = GetTime();
  Context.hierarchyUpdateTime = t1 - t0;
  Context.updateTime = t2 - t1;
}

void Scene::RenderBegin() {
  float t0 = GetTime();
  GetSystemInstance<RenderSystem>()->RenderBegin();
  // enable the scripts to draw something in the scene
  float t1 = GetTime();
  GetSystemInstance<AnimationSystem>()->Render();
  GetSystemInstance<NativeScriptSystem>()->DrawToScene();
  float t2 = GetTime();

  Context.renderTime = t1 - t0;
  Context.debugDrawTime = t2 - t1;
}

void Scene::RenderEnd() { GetSystemInstance<RenderSystem>()->RenderEnd(); }

void Scene::Reset() {
  // reset entities
  for (auto root : HierarchyRoots)
    DestroyEntity(root->ID);
  GetComponentList<aEngine::Camera>()->data.clear();
  GetComponentList<aEngine::Light>()->data.clear();
  GetComponentList<aEngine::MeshRenderer>()->data.clear();
  GetComponentList<aEngine::NativeScript>()->data.clear();
  HierarchyRoots.clear();
  entitiesSignatures.clear();
  entities.clear();

  // reset scene context
  Context.Reset();
}

void Scene::Destroy() {
  for (auto root : HierarchyRoots)
    DestroyEntity(root->ID);
  HierarchyRoots.clear();
  entitiesSignatures.clear();
  entities.clear();

  // destroy all the systems
  GetSystemInstance<RenderSystem>()->Destroy();
  GetSystemInstance<LightSystem>()->Destroy();
  GetSystemInstance<AnimationSystem>()->Destroy();
  GetSystemInstance<NativeScriptSystem>()->Destroy();
}

bool Scene::LoopCursorInSceneWindow() {
  glm::vec2 cursorPos = Context.currentMousePosition;
  if (!InSceneWindow(cursorPos.x, cursorPos.y)) {
    cursorPos -= Context.sceneWindowPos;
    while (cursorPos.x < 0.0f)
      cursorPos.x += Context.sceneWindowSize.x;
    while (cursorPos.x > Context.sceneWindowSize.x)
      cursorPos.x -= Context.sceneWindowSize.x;
    while (cursorPos.y < 0.0f)
      cursorPos.y += Context.sceneWindowSize.y;
    while (cursorPos.y > Context.sceneWindowSize.y)
      cursorPos.y -= Context.sceneWindowSize.y;
    cursorPos += Context.sceneWindowPos;
    glfwSetCursorPos(Context.window, cursorPos.x, cursorPos.y);
    return false;
  } else
    return true;
}

bool Scene::GetActiveCamera(EntityID &camera) {
  if (!Context.hasActiveCamera) {
    camera = (EntityID)(-1);
    return false;
  } else {
    camera = Context.activeCamera;
    return true;
  }
}

bool Scene::SetActiveCamera(EntityID camera) {
  if (HasComponent<Camera>(camera)) {
    Context.activeCamera = camera;
    Context.hasActiveCamera = true;
    return true;
  } else
    return false;
}

std::shared_ptr<Entity> Scene::AddNewEntity() {
  const EntityID id = addNewEntity();
  entities.insert(std::make_pair(id, std::make_shared<Entity>(id)));
  entities[id]->name += std::to_string(id);
  return entities[id];
}

bool Scene::EntityValid(EntityID &id) {
  auto it = entities.find(id);
  if (it == entities.end()) {
    // this entity is not valid
    id = (EntityID)(-1);
    return false;
  } else
    return true;
}

std::shared_ptr<Entity> Scene::EntityFromID(const EntityID entity) {
  if (entity == (EntityID)(-1)) {
    LOG_F(ERROR, "can't get entity index -1");
    return nullptr;
  }
  if (entity >= MAX_ENTITY_COUNT)
    throw std::runtime_error(
        "EntityID out of range (MAX_ENTITY_COUNT) during EntityFromID");
  if (entities.count(entity) == 0) {
    LOG_F(ERROR, "No entity match this id: %ld", entity);
    return nullptr;
  } else {
    return entities.at(entity);
  }
}

void Scene::DestroyEntity(const EntityID entity) {
  if (entity == Context.activeCamera) {
    LOG_F(INFO, "can't remove active camera");
    return;
  }
  if (entity >= MAX_ENTITY_COUNT)
    throw std::runtime_error("Destroying entity out of range");
  if (entitiesSignatures.find(entity) == entitiesSignatures.end())
    throw std::runtime_error("Destroying entity do not exists");
  // use pointers temporary
  auto ptr = entities[entity].get();
  std::stack<Entity *> s1, s2;
  s1.push(ptr);

  auto handleDestroy = [&](EntityID id) {
    entitiesSignatures.erase(id);
    entities.erase(id);
    for (auto &array : componentsArrays) {
      array.second->Erase(id);
    }
    // the destroyed entity won't appear in any systems
    for (auto &system : registeredSystems) {
      system.second->RemoveEntity(id);
    }
    entityCount--;
    availableEntities.push(id);
  };

  while (!s1.empty()) {
    auto cur = s1.top();
    s2.push(cur);
    s1.pop();
    for (auto child : cur->children)
      s1.push(child);
  }
  while (!s2.empty()) {
    auto cur = s2.top();
    // destroy parent child relation before the entity is deconstructed
    cur->Destroy();
    handleDestroy(cur->ID);
    s2.pop();
  }
}

void Scene::recomputeLocalAxis() {
  for (auto entity : entities) {
    entity.second->UpdateLocalAxis();
  }
}

void Scene::rebuildHierarchyStructure() {
  // clear HierarchyRoots at the start
  HierarchyRoots.clear();
  std::queue<std::pair<Entity *, bool>> q;
  for (auto entity : entities) {
    if (entity.second->parent == nullptr) {
      q.push(
          std::make_pair(entity.second.get(), entity.second->transformDirty));
      HierarchyRoots.push_back(entity.second.get());
    }
  }
  // traverse the entities in a parent first fashion
  // int counter = 0;
  // static int globalCounter = 0;
  while (!q.empty()) {
    auto [ent, dirty] = q.front();
    q.pop();
    for (auto child : ent->children)
      q.push(std::make_pair(child, dirty || child->transformDirty));
    // update global positions with local positions
    if (dirty) {
      // counter++;
      ent->m_position = ent->LocalToGlobal(ent->localPosition);
      ent->m_rotation = ent->GetParentOrientation() * ent->localRotation;
      ent->m_scale = ent->GetParentScale() * ent->localScale;
      ent->transformDirty = false;
    }
  }
  // if (counter > 1)
  //   std::cout << globalCounter++ << ": update " << counter << " times one
  //   frame" << std::endl;
}

// Serialize current scene to a json file
Json Scene::Serialize() {
  Json json;
  // the entity data
  for (auto entity : entities) {
    std::string currentEntityID = std::to_string((unsigned int)entity.first);
    entity.second->Serialize(json["entities"][currentEntityID]);
  }
  // // if more components created, they need to be registered here to be
  // // serialized
  // for (auto camera : GetComponentList<Camera>()->data) {
  //   json["components"]["camera"][std::to_string(camera.GetID())] =
  //       camera.Serialize();
  // }
  // for (auto light : GetComponentList<Light>()->data) {
  //   json["components"]["light"][std::to_string(light.GetID())] =
  //       light.Serialize();
  // }
  // for (auto renderer : GetComponentList<MeshRenderer>()->data) {
  //   json["components"]["renderer"][std::to_string(renderer.GetID())] =
  //       renderer.Serialize();
  // }

  // scene specific settings
  EntityID camera;
  if (GetActiveCamera(camera))
    json["scene"]["activeCamera"] = (int)camera;
  else
    json["scene"]["activeCamera"] = -1;
  return json;
}

// Reset current scene from a json file
void Scene::DeserializeReset(Json &json) {
  // reset current scene
  Reset();

  // reload scene from the file
  std::map<EntityID, EntityID> old2new;
  std::map<EntityID, std::vector<EntityID>> childrenMap;
  std::queue<EntityID> q;
  for (auto entJson : json["entities"].items()) {
    EntityID old = (EntityID)std::stoi(entJson.key());
    auto newEntity = AddNewEntity();
    newEntity->name = entJson.value()["name"];
    old2new[old] = newEntity->ID;
    EntityID oldParentID =
        entJson.value().value("parent", "none") == "none"
            ? -1
            : (EntityID)std::stoi(entJson.value().value("parent", "none"));
    if (oldParentID != (EntityID)(-1)) {
      childrenMap[oldParentID].push_back(old);
    } else {
      q.push(old); // keep record of potential root nodes
      childrenMap[old] = std::vector<EntityID>();
    }
  }
  std::vector<EntityID> traversalOrder;
  while (!q.empty()) {
    auto cur = q.front();
    q.pop();
    traversalOrder.push_back(cur);
    if (childrenMap.find(cur) != childrenMap.end()) {
      for (auto c : childrenMap[cur]) {
        q.push(c);
      }
    }
  }

  // update the parent first, make sure the hierarchy positions are correct
  for (auto currentUpdateIndex : traversalOrder) {
    // auto entJson = json["entities"][std::to_string((int)currentUpdateIndex)];
    // EntityID old = currentUpdateIndex;
    // EntityID newID = old2new[old];
    // auto newEntity = EntityFromID(newID);
    // newEntity->SetGlobalPosition(entJson["p"].get<glm::vec3>());
    // newEntity->SetGlobalScale(entJson["s"].get<glm::vec3>());
    // newEntity->SetGlobalRotation(entJson["r"].get<glm::quat>());
    // if (entJson.value("parent", "none") != "none") {
    //   auto oldParentID = (EntityID)std::stoi(entJson.value("parent",
    //   "none")); auto parent = EntityFromID(old2new[oldParentID]);
    //   parent->AssignChild(newEntity);
    // }
    // // reload the components
    // for (auto compJson : json["components"]["camera"].items()) {
    //   EntityID belongs = (EntityID)std::stoi(compJson.key());
    //   if (belongs == old) {
    //     // this component belongs to the entity
    //     newEntity->AddComponent<Camera>();
    //     newEntity->GetComponent<Camera>().Deserialize(
    //         json["components"]["camera"][std::to_string((int)old)]);
    //   }
    // }
    // for (auto compJson : json["components"]["light"].items()) {
    //   EntityID belongs = (EntityID)std::stoi(compJson.key());
    //   if (belongs == old) {
    //     // this component belongs to the entity
    //     newEntity->AddComponent<Light>();
    //     newEntity->GetComponent<Light>().Deserialize(
    //         json["components"]["light"][std::to_string((int)old)]);
    //   }
    // }
    // for (auto compJson : json["components"]["renderer"].items()) {
    //   EntityID belongs = (EntityID)std::stoi(compJson.key());
    //   if (belongs == old) {
    //     // this component belongs to the entity
    //     newEntity->AddComponent<MeshRenderer>();
    //     newEntity->GetComponent<MeshRenderer>().Deserialize(
    //         json["components"]["renderer"][std::to_string((int)old)]);
    //   }
    // }
  }

  // reset the scene variables
  if (json["scene"]["activeCamera"].get<int>() != -1)
    SetActiveCamera(old2new[json["scene"]["activeCamera"]]);
}

}; // namespace aEngine
