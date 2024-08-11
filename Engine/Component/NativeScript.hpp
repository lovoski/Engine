#pragma once

#include "Base/BaseComponent.hpp"
#include "Component/Scriptable.hpp"

namespace aEngine {

struct NativeScript : public aEngine::BaseComponent {

  // Bind a scriptable object to current component
  template <typename T> void Bind() {
    // create an instance of the scriptable
    instance = new T();
    // set up entity for this script instance
    instance->entity = GWORLD.EntityFromID(entityID);
    instance->Start();
  }

  template <typename T> void Unbind() {
    if (instance) {
      instance = static_cast<T *>(instance);
      instance->Destroy();
      delete instance;
      instance = nullptr;
    }
  }

  // A scriptable is attached to a native script component,
  // which is located on a actual entity
  Scriptable *instance;
};

}; // namespace aEngine