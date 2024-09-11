#pragma once

#include "Global.hpp"
#include "Base/Types.hpp"

namespace aEngine {

class BaseComponent {
public:
  BaseComponent(EntityID id) : entityID(id) {}
  virtual ~BaseComponent() = default;
  const EntityID GetID() const { return entityID; }

  virtual void DrawInspectorGUI() {}

  static std::size_t HashString(std::string str);

protected:
  friend class Scene;
  EntityID entityID;
};

}; // namespace aEngine