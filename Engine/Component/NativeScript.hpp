#pragma once

#include "Base/BaseComponent.hpp"
#include "Base/Scriptable.hpp"
#include "Scene.hpp"

namespace aEngine {

struct NativeScript : public aEngine::BaseComponent {
  NativeScript() : BaseComponent(-1) {}
  NativeScript(EntityID id) : BaseComponent(id) {}

  ~NativeScript();

  // Bind a scriptable object to current component
  template <typename T> void Bind() {
    static_assert((std::is_base_of<Scriptable, T>::value &&
                   !std::is_same<Scriptable, T>::value),
                  "Invalid template type for NativeScript");
    // create an instance of the scriptable
    auto sid = ScriptableType<T>();
    if (ScriptMap.find(sid) == ScriptMap.end()) {
      // register the script
      ScriptMap.insert(std::make_pair(sid, std::make_unique<T>()));
    }
    if (instances.find(sid) != instances.end()) {
      // overwrite scriptable object
      LOG_F(INFO, "overwrite existing scriptable %s", typeid(T).name());
      Unbind<T>();
    }
    Scriptable *instance = new T();
    // set up entity for this script instance
    instance->entity = GWORLD.EntityFromID(entityID).get();
    instance->Start();
    instance->OnEnable();
    instances[sid] = instance;
  }

  template <typename T> void Unbind() {
    static_assert((std::is_base_of<Scriptable, T>::value &&
                   !std::is_same<Scriptable, T>::value),
                  "Invalid template type for NativeScript");
    auto sid = ScriptableType<T>();
    if (instances.find(sid) == instances.end()) {
      // unbind a scriptable that don't exists
      LOG_F(ERROR, "unbind a scriptable don't exist %s", typeid(T).name());
    } else {
      auto instance = static_cast<T *>(instances[sid]);
      instance->OnDisable();
      instance->Destroy();
      if (instance)
        delete instance;
      instances.erase(sid);
    }
  }

  void DrawInspectorGUI() override;

  // A scriptable component can hold multiple scriptable objects
  std::map<ScriptableTypeID, Scriptable *> instances;

  // map scriptable id to scriptable instance
  static std::map<ScriptableTypeID, std::unique_ptr<Scriptable>> ScriptMap;

private:
  // New scripts should be manually registered here
  void drawAddScriptPopup();
};

}; // namespace aEngine