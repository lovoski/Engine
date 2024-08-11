#pragma once

#include "Base/BaseComponent.hpp"
#include "Component/Scriptable.hpp"

namespace aEngine {

struct NativeScript : public aEngine::BaseComponent {

  // Bind a scriptable object to current component
  // TODO: there could be some better ways to do it,
  // passing the pointer of the entity feel not right
  template <typename T> void Bind(Entity *entity) {
    // create an instance of the scriptable
    instance = new T();
    // set up entity for this script instance
    instance->entity = entity;
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