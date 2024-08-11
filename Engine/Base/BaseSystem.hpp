#pragma once

#include "Base/Types.hpp"

namespace aEngine {

class Scene;

class BaseSystem {
public:
  BaseSystem() = default;
  virtual ~BaseSystem() = default;

  void RemoveEntity(const EntityID entity) { entities.erase(entity); }

  void AddEntity(const EntityID entity) { entities.insert(entity); }

  const EntitySignature GetSignature() const { return signature; }

  // add some component to a system, call this function in the overriden
  // constructor
  template <typename T> void AddComponentSignature() {
    signature.insert(ComponentType<T>());
  }

  virtual void Start() {}
  virtual void Update() {}
  virtual void LateUpdate() {}
  virtual void Render() {}
  virtual void Destroy() {}

protected:
  EntitySignature signature;
  std::set<EntityID> entities;
};

}; // namespace aEngine