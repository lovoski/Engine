/**
 * To have a working native script in the scene, the script should
 * derive from this base class and implement the virtual functions in it.
 */
#pragma once

#include "Types.hpp"

// refer to REGISTER_RENDER_PASS
#define REGISTER_SCRIPT(Namespace, ScriptType)                                 \
  CEREAL_REGISTER_TYPE(Namespace::ScriptType);                                 \
  CEREAL_REGISTER_POLYMORPHIC_RELATION(aEngine::Scriptable,                    \
                                       Namespace::ScriptType)

// refer to REGISTER_RENDER_PASS_SL
#define REGISTER_SCRIPT_SL(Namespace, ScriptType)                              \
  CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(aEngine::Scriptable,                      \
                                     cereal::specialization::member_load_save) \
  REGISTER_SCRIPT(Namespace, ScriptType)

namespace aEngine {

class Entity;

class Scriptable {
public:
  // Don't do any operations involving reading the entity member variable inside
  // the constructor, this variable is only set after the constructor being
  // called. See `NativeScipt` component's `Bind` function for more details.
  Scriptable() {}
  ~Scriptable() {}

  // The start function only get called once
  // when the script is attached to some entity
  // with the Bind function
  virtual void Start() {}

  // This function will gets called when switch from `disable` to `enable`
  // at the inspector gui, this function will also gets called once after
  // the start function.
  virtual void OnEnable();
  // This function will gets called when switch from `enable` to `disable`
  // at the inspector gui
  virtual void OnDisable();

  virtual void Update(float dt) {}
  // LateUpdate will be called after all
  // Update functions are called
  virtual void LateUpdate(float dt) {}

  // The destroy function is also called once
  // when the script instance is dettached from a entity
  virtual void Destroy() {}

  // We can draw debug helpers with this function
  virtual void DrawToScene() {}

  virtual std::string getInspectorWindowName();

  // don't override this function
  void DrawInspectorGUIInternal();

  bool Enabled = true;

  Entity *entity = nullptr;

  template <typename Archive> void serialize(Archive &ar) {}

private:
  // override this function
  virtual void DrawInspectorGUI() {}
};

}; // namespace aEngine
