#include "Scene.hpp"
#include "Engine.hpp"

#include "Component/Camera.hpp"
#include "Component/Light.hpp"
#include "Component/Mesh.hpp"
#include "Component/MeshRenderer.hpp"
#include "Component/NativeScript.hpp"

#include "System/Animation/AnimationSystem.hpp"
#include "System/Audio/AudioSystem.hpp"
#include "System/NativeScript/NativeScriptSystem.hpp"
#include "System/Render/CameraSystem.hpp"
#include "System/Render/LightSystem.hpp"
#include "System/Render/RenderSystem.hpp"
#include "System/Spatial/SpatialSystem.hpp"

#include "Scripts/CameraController.hpp"

namespace aEngine {

Scene::Scene() {
  // create the entities
  entityCount = 0;
  // entityID = 0 is considered an invalid entity
  for (EntityID entity = 1; entity <= MAX_ENTITY_COUNT; ++entity) {
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
  RegisterSystem<CameraSystem>();
  RegisterSystem<LightSystem>();
  RegisterSystem<AnimationSystem>();
  RegisterSystem<NativeScriptSystem>();
  RegisterSystem<SpatialSystem>();
  RegisterSystem<AudioSystem>();

  // start all the systems
  for (auto &sys : registeredSystems)
    sys.second->Start();
}

void Scene::resetVariables() {
  // clear rotation update events
  Entity::InternalRotationUpdate.clear();
}

void Scene::Update() {
  resetVariables();
  // tick the timer
  Context.Tick();
  float t0 = GetTime();
  // update the transforms first
  recomputeLocalAxis();
  rebuildHierarchyStructure();
  float t1 = GetTime();

  // pre-update the readonly variables for Update
  for (auto &system : registeredSystems)
    system.second->PreUpdate(Context.deltaTime);
  // the main update for all systems
  for (auto &system : registeredSystems)
    system.second->Update(Context.deltaTime);

  // call late update
  GetSystemInstance<NativeScriptSystem>()->LateUpdate(Context.deltaTime);
  float t2 = GetTime();
  Context.hierarchyUpdateTime = t1 - t0;
  Context.updateTime = t2 - t1;

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    LOG_F(ERROR, "Framebuffer is not complete");

  // do the rendering
  ForceRender();
}

void Scene::ForceRender() {
  float t3 = GetTime();
  GetSystemInstance<RenderSystem>()->Render();
  // enable the scripts to draw something in the scene
  float t4 = GetTime();
  GetSystemInstance<AudioSystem>()->Render();
  GetSystemInstance<CameraSystem>()->Render();
  GetSystemInstance<LightSystem>()->Render();
  GetSystemInstance<AnimationSystem>()->Render();
  GetSystemInstance<SpatialSystem>()->Render();
  GetSystemInstance<NativeScriptSystem>()->DrawToScene();
  float t5 = GetTime();

  Context.renderTime = t4 - t3;
  Context.debugDrawTime = t5 - t4;
}

void Scene::RenderEnd() { GetSystemInstance<RenderSystem>()->RenderEnd(); }

void Scene::SetupDefaultScene() {
  auto ent = AddNewEntity();
  ent->name = "Script Base";
  ent->AddComponent<aEngine::NativeScript>();
  ent->GetComponent<aEngine::NativeScript>()->Bind<EditorCameraController>();

  auto cam = AddNewEntity();
  cam->name = "Editor Cam";
  cam->AddComponent<Camera>();
  auto camera = cam->GetComponent<Camera>();
  camera->zFar = 2000.0f;
  cam->SetGlobalPosition(glm::vec3(0.0f, 3.0f, 5.0f));
  SetActiveCamera(cam->ID);

  auto skybox = AddNewEntity();
  skybox->name = "Environment Light";
  skybox->AddComponent<EnvironmentLight>();
}

void Scene::Reset() {
  // reset entities
  for (auto root : HierarchyRoots)
    DestroyEntity(root->ID);
  GetComponentList<Camera>()->data.clear();
  GetComponentList<DirectionalLight>()->data.clear();
  GetComponentList<PointLight>()->data.clear();
  GetComponentList<EnvironmentLight>()->data.clear();
  GetComponentList<Animator>()->data.clear();
  GetComponentList<DeformRenderer>()->data.clear();
  GetComponentList<MeshRenderer>()->data.clear();
  GetComponentList<NativeScript>()->data.clear();
  GetComponentList<Mesh>()->data.clear();
  HierarchyRoots.clear();
  entitiesSignatures.clear();
  entities.clear();

  // reset scene context
  Context.Reset();
  // reset system context
  for (auto system : registeredSystems) {
    system.second->Reset();
  }
}

void Scene::PlotSceneProfile() {
  static float timeCounter = 0.0f;
  static float mainRenderTime = 0.0f, displayMainRenderTime = 0.0f;
  static float mainUpdateTime = 0.0f, displayMainUpdateTime = 0.0f;
  static float debugRenderTime = 0.0f, displayDebugRenderTime = 0.0f;
  static float hierarchyUpdateTime = 0.0f, displayHierarchyUpdateTime = 0.0f;
  static int frameCounter = 0, displayFPS = 0;
  timeCounter += Context.deltaTime;
  mainUpdateTime += Context.updateTime;
  mainRenderTime += Context.renderTime;
  debugRenderTime += Context.debugDrawTime;
  hierarchyUpdateTime += Context.hierarchyUpdateTime;
  frameCounter++;
  if (timeCounter >= 0.5f) {
    displayFPS = frameCounter * 2;
    displayMainRenderTime = mainRenderTime / frameCounter;
    displayMainUpdateTime = mainUpdateTime / frameCounter;
    displayDebugRenderTime = debugRenderTime / frameCounter;
    displayHierarchyUpdateTime = hierarchyUpdateTime / frameCounter;
    mainRenderTime = 0.0f;
    mainUpdateTime = 0.0f;
    debugRenderTime = 0.0f;
    hierarchyUpdateTime = 0.0f;
    frameCounter = 0;
    timeCounter = 0.0f;
  }
  ImGui::SeparatorText("Time");
  ImGui::MenuItem("Frames Per Second:", nullptr, nullptr, false);
  ImGui::Text("%d", displayFPS);
  ImGui::MenuItem("Hierarchy Update:", nullptr, nullptr, false);
  ImGui::Text("%.4f ms", displayHierarchyUpdateTime * 1000);
  ImGui::MenuItem("Main Update:", nullptr, nullptr, false);
  ImGui::Text("%.4f ms", displayMainUpdateTime * 1000);
  ImGui::MenuItem("Main Render:", nullptr, nullptr, false);
  ImGui::Text("%.4f ms", displayMainRenderTime * 1000);
  ImGui::MenuItem("Debug Render:", nullptr, nullptr, false);
  ImGui::Text("%.4f ms", displayDebugRenderTime * 1000);
  ImGui::MenuItem("Delta Time:", nullptr, nullptr, false);
  ImGui::Text("%.4f ms", 1000.0f / displayFPS);

  ImGui::SeparatorText("Objects");
  ImGui::MenuItem("Active Camera:", nullptr, nullptr, false);
  ImGui::Text("Entity ID: %d",
              Context.hasActiveCamera ? Context.activeCamera : -1);
  ImGui::Text("Entity Name: %s",
              !Context.hasActiveCamera
                  ? "None"
                  : EntityFromID(Context.activeCamera)->name.c_str());

  ImGui::MenuItem("Entities", nullptr, nullptr, false);
  ImGui::Text("Entity Counter: %d", entityCount);
  ImGui::Text("Animation Entities: %d",
              GetSystemInstance<AnimationSystem>()->GetNumEntities());
  ImGui::Text("Render Entities: %d",
              GetSystemInstance<RenderSystem>()->GetNumEntities());
  ImGui::Text("Camera Entities: %d",
              GetSystemInstance<CameraSystem>()->GetNumEntities());
  ImGui::Text("Lights Entities: %d",
              GetSystemInstance<LightSystem>()->GetNumEntities());
  ImGui::Text("Script Entities: %d",
              GetSystemInstance<NativeScriptSystem>()->GetNumEntities());

  ImGui::SeparatorText("Registered Types");
  static bool queryRegisteredTypes = false;
  ImGui::Checkbox("Enable Query", &queryRegisteredTypes);
  if (queryRegisteredTypes) {
    ImGui::MenuItem("Systems", nullptr, nullptr, false);
    for (auto &system : registeredSystems) {
      ImGui::Text("%s", typeid(*system.second.get()).name());
    }
    ImGui::MenuItem("Components", nullptr, nullptr, false);
    for (auto &compPair : BaseSystem::CompMap) {
      ImGui::Text("%d:%s", compPair.first,
                  typeid(*compPair.second.get()).name());
    }
    ImGui::MenuItem("Native Scripts", nullptr, nullptr, false);
    for (auto &scriptPair : NativeScript::ScriptMap) {
      ImGui::Text("%d:%s", scriptPair.first,
                  typeid(*scriptPair.second.get()).name());
    }
  }
}

void Scene::Destroy() {
  for (auto root : HierarchyRoots)
    DestroyEntity(root->ID);
  HierarchyRoots.clear();
  entitiesSignatures.clear();
  entities.clear();

  // destroy all the systems
  for (auto &sys : registeredSystems)
    sys.second->Destroy();
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
    camera = (EntityID)(0);
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
    id = (EntityID)(0);
    return false;
  } else
    return true;
}

std::shared_ptr<Entity> Scene::EntityFromID(const EntityID entity) {
  if (entity == (EntityID)(0)) {
    LOG_F(ERROR, "can't get entity index 0");
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
    LOG_F(WARNING, "remove active camera on the scene");
    Context.activeCamera = (EntityID)(0);
    Context.hasActiveCamera = false;
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

bool Scene::BelongToSystem(const EntityID entity,
                           const EntitySignature &systemSignature,
                           const EntitySignature &systemSignatureOne) {
  // if the entity has at least one of the signatures in the
  // systemSignatureOne it will get updated by this system
  auto entitySignature = GetEntitySignature(entity);
  bool requireOneSatisfied = false;
  if (systemSignatureOne.size() > 0) {
    for (const auto compType : systemSignatureOne) {
      if (entitySignature->count(compType) == 1) {
        requireOneSatisfied = true;
        break;
      }
    }
    if (!requireOneSatisfied)
      return false;
  }
  // Assume the RequireOne condition is met
  for (const auto compType : systemSignature) {
    if (entitySignature->count(compType) == 0) {
      return false;
    }
  }
  // The entity also has all components registered with RequireAll
  return true;
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
  while (!q.empty()) {
    auto [ent, dirty] = q.front();
    q.pop();
    for (auto child : ent->children)
      q.push(std::make_pair(child, dirty || child->transformDirty));
    // update global positions with local positions
    if (dirty) {
      // counter++;
      ent->m_position = ent->LocalToGlobal(ent->localPosition);
      // the global rotation is constructed from localRotation
      // all dirty children will get this updated parent orientation
      ent->m_rotation = ent->GetParentOrientation() * ent->localRotation;
      ent->m_scale = ent->GetParentScale() * ent->localScale;
      ent->UpdateGlobalTransform();
      ent->transformDirty = false;
    }
  }
}

bool Scene::Save(std::string path) {
  std::ofstream output(path, std::ios::binary);
  if (output.is_open()) {
    LOG_F(INFO, "save scene to %s", path.c_str());
    cereal::JSONOutputArchive oa(output);
    oa(CEREAL_NVP(Context));
    // 1. Entities
    // the parent-child relation is not serialized here
    oa(CEREAL_NVP(entityCount), CEREAL_NVP(entities),
       CEREAL_NVP(entitiesSignatures));
    // collect parent-child relation for all entities
    std::map<EntityID, EntityID> parentSerialize;
    std::map<EntityID, std::vector<EntityID>> childrenSerialize;
    for (auto &entity : entities) {
      if (entity.second->parent != nullptr)
        parentSerialize[entity.second->ID] = entity.second->parent->ID;
      else
        parentSerialize[entity.second->ID] = 0; // set parent to null entity
      childrenSerialize[entity.second->ID] = std::vector<EntityID>();
      auto &cref = childrenSerialize[entity.second->ID];
      for (auto child : entity.second->children) {
        cref.push_back(child->ID);
      }
    }
    // serialize parent child entity
    oa(CEREAL_NVP(parentSerialize), CEREAL_NVP(childrenSerialize));
    // hierarchy roots
    std::vector<EntityID> roots;
    for (auto r : HierarchyRoots)
      roots.push_back(r->ID);
    oa(roots);

    // 2. Components
    oa(CEREAL_NVP(componentsArrays));

    // 3. Systems
    oa(CEREAL_NVP(registeredSystems));

    // 4. Assets
    oa(Loader.allMaterials);

    return true;
  } else {
    LOG_F(ERROR, "failed to create scene file %s", path.c_str());
    return false;
  }
}

bool Scene::Load(std::string path) {
  std::ifstream input(path, std::ios::binary);
  if (input.is_open()) {
    // reset current scene
    Reset();
    LOG_F(INFO, "load scene from %s", path.c_str());
    cereal::JSONInputArchive ia(input);
    ia(CEREAL_NVP(Context));
    // 1. Entities
    ia(CEREAL_NVP(entityCount), CEREAL_NVP(entities),
       CEREAL_NVP(entitiesSignatures));
    while (!availableEntities.empty())
      availableEntities.pop();
    for (EntityID entID = 1; entID <= MAX_ENTITY_COUNT; ++entID)
      if (entities.count(entID) == 0)
        availableEntities.push(entID);

    std::map<EntityID, EntityID> parentSerialize;
    std::map<EntityID, std::vector<EntityID>> childrenSerialize;
    // get parent-child relation
    ia(CEREAL_NVP(parentSerialize), CEREAL_NVP(childrenSerialize));
    // restore parent-child relation
    for (auto &entity : entities) {
      auto id = entity.second->ID;
      auto parentId = parentSerialize[id];
      auto childrenId = childrenSerialize[id];
      if (parentId == (EntityID)(0)) {
        entity.second->parent = nullptr;
      } else {
        entity.second->parent = entities[parentId].get();
      }
      entity.second->children.clear();
      for (auto child : childrenId) {
        entity.second->children.push_back(entities[child].get());
      }
    }
    // hierarchy root
    std::vector<EntityID> roots;
    ia(roots);
    HierarchyRoots.clear();
    for (auto id : roots)
      HierarchyRoots.push_back(entities[id].get());

    // 2. Components
    ia(CEREAL_NVP(componentsArrays));

    // 3. Systems
    ia(CEREAL_NVP(registeredSystems));

    // 4. Assets
    ia(Loader.allMaterials);

    return true;
  } else {
    LOG_F(ERROR, "failed to load scene from %s", path.c_str());
    return false;
  }
}

}; // namespace aEngine
