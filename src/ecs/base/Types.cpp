#include "Types.hpp"

namespace ECS {

const ComponentTypeID GetRuntimeComponentTypeID() {
  static ComponentTypeID typeID = 0u;
  return typeID++;
}

const SystemTypeID GetRuntimeSystemTypeID() {
  static SystemTypeID typeID = 0u;
  return typeID++;
}

};