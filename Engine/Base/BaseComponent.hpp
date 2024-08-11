#pragma once

#include "Global.hpp"
#include "Base/Types.hpp"

namespace aEngine {

class BaseComponent {
public:
  BaseComponent() : entityID() {}
  virtual ~BaseComponent() = default;
  const EntityID GetID() const { return entityID; }

  virtual Json Serialize() { return Json(); }
  virtual void Deserialize(Json &json) {}

protected:
  friend class Scene;
  EntityID entityID;
};

}; // namespace aEngine