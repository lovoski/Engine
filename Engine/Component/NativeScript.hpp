#pragma once

#include "Base/BaseComponent.hpp"
#include "Base/Scriptable.hpp"
#include "Scene.hpp"

namespace aEngine {

struct NativeScript : public aEngine::BaseComponent {

  // Bind a scriptable object to current component
  template <typename T> void Bind() {
    // create an instance of the scriptable
    auto sid = ScriptableType<T>();
    if (instances.find(sid) != instances.end()) {
      // overwrite scriptable object
      Console.Log("[info]: overwrite existing scriptable %s\n", typeid(T).name());
      Unbind<T>();
    }
    Scriptable *instance = new T();
    // set up entity for this script instance
    instance->entity = GWORLD.EntityFromID(entityID);
    instance->Start();
    instances[sid] = instance;
  }

  template <typename T> void Unbind() {
    auto sid = ScriptableType<T>();
    if (instances.find(sid) == instances.end()) {
      // unbind a scriptable that don't exists
      Console.Log("[error]: unbind a scriptable don't exist %s\n", typeid(T).name());
    } else {
      auto instance = static_cast<T *>(instances[sid]);
      instance->Destroy();
      if (instance)
        delete instance;
      instances.erase(sid);
    }
  }

  void DrawInspectorGUI() override {
    if (ImGui::TreeNode("Native Script")) {
      for (auto instance : instances) {
        if (instance.second != nullptr) {
          // if this is a valid scriptable
          instance.second->DrawInspectorGUI();
        }
      }
      ImGui::TreePop();
    }
  }

  // A scriptable component can hold multiple scriptable objects
  std::map<ScriptableTypeID, Scriptable*> instances;
};

}; // namespace aEngine