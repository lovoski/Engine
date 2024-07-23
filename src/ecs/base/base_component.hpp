#pragma once

#include "types.hpp"

namespace ECS {

class BaseComponent {
public:
  BaseComponent() : entityID() {}
  virtual ~BaseComponent() = default;
  const EntityID GetID() const { return entityID; }

protected:
  friend class EntityManager;
  EntityID entityID;
};

}; // namespace ECS