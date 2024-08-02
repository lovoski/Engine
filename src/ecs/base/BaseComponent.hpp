#pragma once

#include "Types.hpp"

namespace ECS {

class BaseComponent {
public:
  BaseComponent() : entityID() {}
  virtual ~BaseComponent() = default;
  const EntityID GetID() const { return entityID; }

  virtual void Serialize(Json &json) {}
  virtual void Deserialize(Json &json) {}

protected:
  friend class EntityManager;
  EntityID entityID;
};

}; // namespace ECS