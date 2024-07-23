#pragma once

#include "types.hpp"

namespace ECS {

class BaseComponent {
public:
  BaseComponent() : entityID() {}
  virtual ~BaseComponent() = default;
  const EntityID GetID() const { return entityID; }

private:
  friend class Manager;
  EntityID entityID;
};

}; // namespace ECS