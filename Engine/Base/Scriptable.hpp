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
  Scriptable() {}
  ~Scriptable() {}

  // The start function only get called once
  // when the script is attached to some entity 
  // with the Bind function
  virtual void Start() {}

  virtual void Update(float dt) {}
  // LateUpdate will be called after all
  // Update functions are called
  virtual void LateUpdate(float dt) {}

  // The destroy function is also called once
  // when the script instance is dettached from a entity
  virtual void Destroy() {}

  // We can draw debug helpers with this function
  virtual void DrawToScene() {}

  // This function would get called in DrawInspectorGUI by default
  // If you override the function DrawInspectorGUI but also want 
  // the default GUI, call this function before your override code
  virtual void drawInspectorGUIDefault();
  virtual std::string getTypeName();

  // The following content will be drawn at the editor menu
  virtual void DrawInspectorGUI();

  bool Enabled = true;

  Entity *entity = nullptr;
};

};
