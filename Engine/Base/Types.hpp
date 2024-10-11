// inline function would be unwrapped into plain code after compilation, these
// functions has only one copy in all hear files
#pragma once

#include <set>
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <algorithm>

namespace aEngine {

class BaseSystem;
class BaseComponent;

const size_t MAX_ENTITY_COUNT = 5000;
const size_t MAX_COMPONENT_COUNT = 128;

using EntityID = size_t;
using SystemTypeID = size_t;
using ComponentTypeID = size_t;
using ScriptableTypeID = size_t;
using EntitySignature = std::set<ComponentTypeID>;

inline ComponentTypeID GetRuntimeComponentTypeID() {
  static ComponentTypeID typeID = 0u;
  return typeID++;
}

inline SystemTypeID GetRuntimeSystemTypeID() {
  static SystemTypeID typeID = 0u;
  return typeID++;
}

inline ScriptableTypeID GetRuntimeScriptableTypeID() {
  static ScriptableTypeID typeID = 0u;
  return typeID++;
}

// attach type id to component class and return it
template <typename T> inline ComponentTypeID ComponentType() noexcept {
  // the class should be inferented from component, but not the class itself
  static_assert((std::is_base_of<BaseComponent, T>::value &&
                 !std::is_same<BaseComponent, T>::value),
                "Invalid template type");
  static const ComponentTypeID typeID = GetRuntimeComponentTypeID();
  return typeID;
}

// attach type id to system class and return it
template <typename T> inline SystemTypeID SystemType() noexcept {
  static_assert((std::is_base_of<BaseSystem, T>::value &&
                 !std::is_same<BaseSystem, T>::value),
                "Invalid template type");
  static const SystemTypeID typeID = GetRuntimeSystemTypeID();
  return typeID;
}

class Scriptable;

// attach type id to scriptable class and return it
template <typename T> inline ScriptableTypeID ScriptableType() noexcept {
  static_assert((std::is_base_of<Scriptable, T>::value &&
                 !std::is_same<Scriptable, T>::value),
                "Invalid template type");
  static const ScriptableTypeID typeID = GetRuntimeScriptableTypeID();
  return typeID;
}

}; // namespace aEngine
