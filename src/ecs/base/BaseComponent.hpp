#pragma once

#include "Types.hpp"

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