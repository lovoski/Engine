#include "System/NativeScript/NativeScriptSystem.hpp"
#include "Component/NativeScript.hpp"
#include "Component/Scriptable.hpp"

namespace aEngine {

NativeScriptSystem::NativeScriptSystem() {
  AddComponentSignature<NativeScript>();
}

void NativeScriptSystem::Update() {
  // update all the entities with a valid script instance
  for (auto entity : entities) {
    auto entityObject = GWORLD.EntityFromID(entity);
    auto nsc = entityObject->GetComponent<NativeScript>();
    // if the script is intancianted and enabled
    if (nsc.instance != nullptr && nsc.instance->Enabled) {
      nsc.instance->Update();
    }
  }
}

void NativeScriptSystem::LateUpdate() {
  // update all the entities with a valid script instance
  for (auto entity : entities) {
    auto entityObject = GWORLD.EntityFromID(entity);
    auto nsc = entityObject->GetComponent<NativeScript>();
    if (nsc.instance != nullptr && nsc.instance->Enabled) {
      nsc.instance->LateUpdate();
    }
  }
}

void NativeScriptSystem::DrawToScene() {
  for (auto entity : entities) {
    auto entityObject = GWORLD.EntityFromID(entity);
    auto nsc = entityObject->GetComponent<NativeScript>();
    if (nsc.instance != nullptr && nsc.instance->Enabled) {
      nsc.instance->DrawToScene();
    }
  }
}

};