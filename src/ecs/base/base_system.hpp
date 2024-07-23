#pragma once

#include "types.hpp"

namespace ECS {

class BaseSystem {
public:
  BaseSystem() = default;
  virtual ~BaseSystem()=default;

  void RemoveEntity(const EntityID entity) {
    entities.erase(entity);
  }

  void AddEntity(const EntityID entity) {
    entities.insert(entity);
  }

  const EntitySignature GetSignature() const {
    return signature;
  }

  template<typename T>
  void AddComponentSignature() {
    signature.insert(ComponentType<T>());
  }

  virtual void Start() {}
  virtual void Update() {}
  virtual void Destroy() {}

private:
  friend class Manager;
  // defines which component this system should take care of
  EntitySignature signature;
  std::set<EntityID> entities;
};

}; // namespace ECS