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

  bool Enabled = true;

  Entity *entity = nullptr;
};

};
