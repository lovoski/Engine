#include "Base/Scriptable.hpp"
#include "Component/NativeScript.hpp"
#include "System/NativeScript/NativeScriptSystem.hpp"
#include "Scene.hpp"

namespace aEngine {

NativeScriptSystem::NativeScriptSystem() {
  AddComponentSignature<NativeScript>();
}

void NativeScriptSystem::Update() {
  // update all the entities with a valid script instance
  for (auto entity : entities) {
    auto entityObject = GWORLD.EntityFromID(entity);
    auto nsc = entityObject->GetComponent<NativeScript>();
    for (auto instance : nsc.instances) {
      if (instance.second != nullptr && instance.second->Enabled) {
        instance.second->Update();
      }
    }
  }
}

void NativeScriptSystem::LateUpdate() {
  // update all the entities with a valid script instance
  for (auto entity : entities) {
    auto entityObject = GWORLD.EntityFromID(entity);
    auto nsc = entityObject->GetComponent<NativeScript>();
    for (auto instance : nsc.instances) {
      if (instance.second != nullptr && instance.second->Enabled) {
        instance.second->LateUpdate();
      }
    }
  }
}

void NativeScriptSystem::DrawToScene() {
  for (auto entity : entities) {
    auto entityObject = GWORLD.EntityFromID(entity);
    auto nsc = entityObject->GetComponent<NativeScript>();
    for (auto instance : nsc.instances) {
      if (instance.second != nullptr && instance.second->Enabled) {
        instance.second->DrawToScene();
      }
    }
  }
}

};