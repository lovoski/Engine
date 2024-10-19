#pragma once

#include "Base/BaseComponent.hpp"
#include "Base/Scriptable.hpp"
#include "Scene.hpp"

namespace aEngine {

class NativeScriptSystem;

struct NativeScript : public aEngine::BaseComponent {
  friend class NativeScriptSystem;

  NativeScript() : BaseComponent(0) {}
  NativeScript(EntityID id) : BaseComponent(id) {}

  ~NativeScript();

  // Bind a scriptable object to current component,
  // each script could have multiple instances registered.
  template <typename T> void Bind() {
    static_assert((std::is_base_of<Scriptable, T>::value &&
                   !std::is_same<Scriptable, T>::value),
                  "Invalid template type for NativeScript");
    instances.push_back(std::make_unique<T>());
    auto &instance = instances[instances.size() - 1];
    // set up entity for this script instance
    instance->entity = GWORLD.EntityFromID(entityID).get();
    instance->Start();
    instance->OnEnable();
  }

  // Unbind the script matching this index.
  bool Unbind(int index) {
    if (index < 0 || index >= instances.size()) {
      LOG_F(ERROR, "index=%d out of range when unbind a script", index);
      return false;
    } else {
      instances[index]->OnDisable();
      instances[index]->Destroy();
      instances.erase(instances.begin() + index);
      return true;
    }
  }

  std::string getInspectorWindowName() override { return "Native Script"; }

  template <typename Archive> void save(Archive &ar) const {
    std::vector<EntityID> ie;
    for (auto &i : instances)
      ie.push_back(i->entity->ID);
    ar(CEREAL_NVP(entityID), instances, ie);
  }
  template <typename Archive> void load(Archive &ar) {
    std::vector<EntityID> ie;
    ar(CEREAL_NVP(entityID), instances, ie);
    for (int i = 0; i < ie.size(); ++i) {
      instances[i]->entity = GWORLD.EntityFromID(ie[i]).get();
    }
  }

  void DrawInspectorGUI() override;

private:
  // A scriptable component can hold multiple scriptable objects
  std::vector<std::unique_ptr<Scriptable>> instances;
  // New scripts should be manually registered here
  void drawAddScriptPopup();
};

}; // namespace aEngine