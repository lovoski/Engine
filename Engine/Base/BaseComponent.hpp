/**
 * Base class for each component,
 * to implement a new component, the component must derive from
 * this BaseComponent, and contains a default constructor.
 * The default constructor don't neccessaryly need to do anything,
 * its just for serialization and reflection need.
 */
#pragma once

#include "Base/ComponentList.hpp"
#include "Base/Types.hpp"
#include "Global.hpp"

namespace aEngine {

class BaseComponent {
public:
  BaseComponent(EntityID id) : entityID(id) {}
  virtual ~BaseComponent() = default;
  const EntityID GetID() const { return entityID; }

  virtual void DrawInspectorGUI() {}

  static std::size_t HashString(std::string str);

  virtual std::string getInspectorWindowName();

  EntityID entityID;
};

}; // namespace aEngine