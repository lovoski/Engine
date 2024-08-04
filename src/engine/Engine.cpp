#include "Engine.hpp"
#include "ecs/systems/render/RenderSystem.hpp"
#include "ecs/systems/camera/CameraSystem.hpp"
#include "ecs/systems/light/LightSystem.hpp"
#include "ecs/systems/gui/GuiSystem.hpp"

// the actual implementation of stb image
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

Engine::Engine()
    : run(true), window(NULL), videoWidth(WIDTH), videoHeight(HEIGHT) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

  auto &monitor = *glfwGetVideoMode(glfwGetPrimaryMonitor());
  glfwWindowHint(GLFW_RED_BITS, monitor.redBits);
  glfwWindowHint(GLFW_BLUE_BITS, monitor.blueBits);
  glfwWindowHint(GLFW_GREEN_BITS, monitor.greenBits);
  glfwWindowHint(GLFW_REFRESH_RATE, monitor.refreshRate);

  window = glfwCreateWindow(videoWidth, videoHeight, "Engine", NULL, NULL);
  if (!window) {
    cout << "Failed to create GLFW window" << endl;
    return;
  }
  glfwMakeContextCurrent(window);
  SceneWindowPos = glm::vec<2, int>(0, 0);
  SceneWindowSize = glm::vec<2, int>((int)videoWidth, (int)videoHeight);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    cout << "Failed to load GLAD" << endl;
    return;
  }

  SceneBuffer = new Graphics::FrameBuffer(videoWidth, videoHeight);
}

Engine::~Engine() {
  EManager.Destroy();
  glfwTerminate();
}

void Engine::Initialize() {
  RManager.Initialize();
  // start and register all the systems
  EManager.Start();
}

void Engine::Reset() {
  // destroy current scene (entities, components)
  for (auto root : EManager.HierarchyRoots)
    EManager.DestroyEntity(root->ID);
  EManager.GetComponentList<Camera>()->data.clear();
  EManager.GetComponentList<Light>()->data.clear();
  EManager.GetComponentList<Material>()->data.clear();
  EManager.GetComponentList<MeshRenderer>()->data.clear();
  EManager.HierarchyRoots.clear();
  EManager.entitiesSignatures.clear();
  EManager.entities.clear();

  // set camera to inactive
  hasActiveCamera = false;
  activeCamera = (ECS::EntityID)(-1);
}

void Engine::Update() {
  EManager.Update();

  // update the scene data if needed
  if (reloadScene) {
    reloadScene = false;
    // the activeSceneFile should be a valid file
    std::ifstream sceneFileInput(ActiveSceneFile);
    Json sceneFileContent;
    sceneFileInput >> sceneFileContent;
    LoadSceneFromJson(sceneFileContent);
    sceneFileInput.close();
  }
}

void Engine::Quit() {
  glfwSetWindowShouldClose(window, GLFW_TRUE);
  run = false;
}

bool Engine::SetActiveCamera(ECS::EntityID camera) {
  if (camera == (ECS::EntityID)(-1)) {
    Console.Log("[info]: camera is not a valid entity, current active camera %d\n", activeCamera);
    return false;
  }
  if (EManager.HasComponent<Camera>(camera)) {
    hasActiveCamera = true;
    activeCamera = camera;
    Console.Log("[info]: set active camera to %s\n", EManager.EntityFromID(camera)->name.c_str());
    return true;
  } else {
    Console.Log("[error]: Not a valid camera entity: %ld\n", camera);
    // there could exist an active camera,
    // don't reset the hasActiveCamera flag
    return false;
  }
}

// get active camera if the flag `hasActiveCamera` is true
bool Engine::GetActiveCamera(ECS::EntityID &camera) {
  if (hasActiveCamera) {
    camera = activeCamera;
    return true;
  } else {
    // Console.Log("[Info]: There's no active camera\n");
    camera = (ECS::EntityID)(-1);
    return false;
  }
}

// check if the scene file is a valid file
// update the variable `activeSceneFile`
void Engine::ReloadScene(string path) {
  std::ifstream sceneFileInput(path);
  if (!sceneFileInput.is_open()) {
    Console.Log("[error]: failed to load scene file from %s\n", path.c_str());
    return;
  } else {
    Console.Log("[info]: load scene file from %s\n", path.c_str());
    ActiveSceneFile = path;
  }
  sceneFileInput.close();
}

bool Engine::LoopCursorInSceneWindow() {
  vec2 cursorPos = Event.MouseCurrentPosition;
  if (!InSceneWindow(cursorPos.x, cursorPos.y)) {
    cursorPos -= SceneWindowPos;
    while (cursorPos.x < 0.0f)
      cursorPos.x += SceneWindowSize.x;
    while (cursorPos.x > SceneWindowSize.x)
      cursorPos.x -= SceneWindowSize.x;
    while (cursorPos.y < 0.0f)
      cursorPos.y += SceneWindowSize.y;
    while (cursorPos.y > SceneWindowSize.y)
      cursorPos.y -= SceneWindowSize.y;
    cursorPos += SceneWindowPos;
    glfwSetCursorPos(&Core.Window(), cursorPos.x, cursorPos.y);
    return false;
  } else
    return true;
}

Json Engine::DumpSceneAsJson() {
  Json json;
  // the entity data
  for (auto entity : EManager.entities) {
    string currentEntityID = std::to_string((unsigned int)entity.first);
    entity.second->Serialize(json["entities"][currentEntityID]);
  }
  // if more components created, they need to be registered here to be serialized
  for (auto camera : EManager.GetComponentList<Camera>()->data) {
    camera.Serialize(
        json["components"]["camera"][std::to_string(camera.GetID())]);
  }
  for (auto material : EManager.GetComponentList<Material>()->data) {
    material.Serialize(json["components"]["material"][std::to_string(material.GetID())]);
  }
  for (auto light : EManager.GetComponentList<Light>()->data) {
    light.Serialize(
        json["components"]["light"][std::to_string(light.GetID())]);
  }
  for (auto renderer : EManager.GetComponentList<MeshRenderer>()->data) {
    renderer.Serialize(
        json["components"]["renderer"][std::to_string(renderer.GetID())]);
  }

  // scene specific settings
  if (GetActiveCamera(activeCamera))
  json["scene"]["activeCamera"] = (int)activeCamera;
  return json;
}

// restore states from a json object
void Engine::LoadSceneFromJson(Json sceneFileContent) {
  // reset current scene
  Reset();

  // reload scene from the file
  std::map<ECS::EntityID, ECS::EntityID> old2new;
  for (auto entJson : sceneFileContent["entities"].items()) {
    ECS::EntityID old = (ECS::EntityID)std::stoi(entJson.key());
    auto newEntity = EManager.AddNewEntity();
    newEntity->name = entJson.value()["name"];
    old2new[old] = newEntity->ID;
  }
  for (auto entJson : sceneFileContent["entities"].items()) {
    // cout << entJson << endl;
    ECS::EntityID old = (ECS::EntityID)std::stoi(entJson.key());
    ECS::EntityID newID = old2new[old];
    auto newEntity = EManager.EntityFromID(newID);
    newEntity->SetGlobalPosition(entJson.value()["p"].get<vec3>());
    newEntity->SetGlobalScale(entJson.value()["s"].get<vec3>());
    newEntity->SetGlobalRotation(entJson.value()["r"].get<vec3>());
    if (entJson.value().value("parent", "none") != "none") {
      auto oldParentID = (ECS::EntityID)std::stoi(entJson.value().value("parent", "none"));
      auto parent = EManager.EntityFromID(old2new[oldParentID]);
      parent->AssignChild(newEntity);
    }
    // reload the components
    for (auto compJson : sceneFileContent["components"]["camera"].items()) {
      ECS::EntityID belongs = (ECS::EntityID)std::stoi(compJson.key());
      if (belongs == old) {
        // this component belongs to the entity
        newEntity->AddComponent<Camera>();
        newEntity->GetComponent<Camera>().Deserialize(sceneFileContent["components"]["camera"][std::to_string((int)old)]);
      }
    }
    for (auto compJson : sceneFileContent["components"]["material"].items()) {
      ECS::EntityID belongs = (ECS::EntityID)std::stoi(compJson.key());
      if (belongs == old) {
        // this component belongs to the entity
        newEntity->AddComponent<Material>();
        newEntity->GetComponent<Material>().Deserialize(sceneFileContent["components"]["material"][std::to_string((int)old)]);
      }
    }
    for (auto compJson : sceneFileContent["components"]["light"].items()) {
      ECS::EntityID belongs = (ECS::EntityID)std::stoi(compJson.key());
      if (belongs == old) {
        // this component belongs to the entity
        newEntity->AddComponent<Light>();
        newEntity->GetComponent<Light>().Deserialize(sceneFileContent["components"]["light"][std::to_string((int)old)]);
      }
    }
    for (auto compJson : sceneFileContent["components"]["renderer"].items()) {
      ECS::EntityID belongs = (ECS::EntityID)std::stoi(compJson.key());
      if (belongs == old) {
        // this component belongs to the entity
        newEntity->AddComponent<MeshRenderer>();
        newEntity->GetComponent<MeshRenderer>().Deserialize(sceneFileContent["components"]["renderer"][std::to_string((int)old)]);
      }
    }
  }
  // reset the scene variables
  if (sceneFileContent["scene"]["activeCamera"].get<int>() != -1)
    SetActiveCamera(old2new[sceneFileContent["scene"]["activeCamera"]]);
}