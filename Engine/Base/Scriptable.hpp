/**
 * To have a working native script in the scene, the script should
 * derive from this base class and implement the virtual functions in it.
 */
#pragma once

#include "Types.hpp"

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
  // at the inspector gui
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

  virtual std::string getTypeName();

  // The following content will be drawn at the editor menu,
  // this function will setup some basic gui components of a scriptable,
  // override the function drawCustomInspectorGUI if you need a 
  // custom InspectorGUI
  void DrawInspectorGUI();

  bool Enabled = true;

  Entity *entity = nullptr;

private:
  // override this function
  virtual void drawCustomInspectorGUI() {}

};

}; // namespace aEngine
