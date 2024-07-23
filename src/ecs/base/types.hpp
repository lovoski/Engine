#pragma once

#include "global.hpp"

namespace ECS {

class BaseSystem;
class BaseComponent;

const size_t MAX_ENTITY_COUNT = 5000;
const size_t MAX_COMPONENT_COUNT = 32;

using EntityID = size_t;
using SystemTypeID = size_t;
using ComponentTypeID = size_t;
using EntitySignature = std::set<ComponentTypeID>;

inline static const ComponentTypeID GetRuntimeComponentTypeID() {
  static SystemTypeID typeID = 0u;
  return typeID++;
}

inline static const SystemTypeID GetRuntimeSystemTypeID() {
  static SystemTypeID typeID = 0u;
  return typeID++;
}

// attach type id to component class and return it
template <typename T> inline static const ComponentTypeID ComponentType() noexcept {
  // the class should be inferented from component, but not the class itself
  static_assert((std::is_base_of<BaseComponent, T>::value &&
                 !std::is_same<BaseComponent, T>::value),
                "Invalid template type");
  static const ComponentTypeID typeID = GetRuntimeComponentTypeID();
  return typeID;
}

// attach type id to system class and return it
template <typename T> inline static const SystemTypeID SystemType() noexcept {
  static_assert(
      (std::is_base_of<BaseSystem, T>::value && !std::is_same<BaseSystem, T>::value),
      "Invalid template type");
  static const SystemTypeID typeID = GetRuntimeSystemTypeID();
  return typeID;
}

}; // namespace ECS
