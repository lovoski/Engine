#include "System/NativeScript/NativeScriptSystem.hpp"
#include "Base/Scriptable.hpp"
#include "Component/NativeScript.hpp"
#include "Entity.hpp"
#include "Scene.hpp"


namespace aEngine {

NativeScriptSystem::NativeScriptSystem() {
  AddComponentSignatureRequireAll<NativeScript>();
}

void NativeScriptSystem::Update(float dt) {
  // update all the entities with a valid script instance
  for (auto entity : entities) {
    auto entityObject = GWORLD.EntityFromID(entity);
    auto nsc = entityObject->GetComponent<NativeScript>();
    for (auto &instance : nsc->instances) {
      if (instance != nullptr && instance->Enabled) {
        instance->Update(dt);
      }
    }
  }
}

void NativeScriptSystem::LateUpdate(float dt) {
  // update all the entities with a valid script instance
  for (auto entity : entities) {
    auto entityObject = GWORLD.EntityFromID(entity);
    auto nsc = entityObject->GetComponent<NativeScript>();
    for (auto &instance : nsc->instances) {
      if (instance != nullptr && instance->Enabled) {
        instance->LateUpdate(dt);
      }
    }
  }
}

void NativeScriptSystem::DrawToScene() {
  for (auto entity : entities) {
    auto entityObject = GWORLD.EntityFromID(entity);
    auto nsc = entityObject->GetComponent<NativeScript>();
    for (auto &instance : nsc->instances) {
      if (instance != nullptr && instance->Enabled) {
        instance->DrawToScene();
      }
    }
  }
}

}; // namespace aEngine

REGISTER_SYSTEM(aEngine, NativeScriptSystem)