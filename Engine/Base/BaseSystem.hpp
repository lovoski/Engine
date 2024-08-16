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

  const EntitySignature GetSignatureOne() const { return signatureOne; }

  // Add some component to a system, the entity must has all component
  // registered with this function
  template <typename T> void AddComponentSignatureRequireAll() {
    signature.insert(ComponentType<T>());
  }

  // Add some component to a system, the entity must has at least one of the 
  // component registered with this function
  template <typename T> void AddComponentSignatureRequireOne() {
    signatureOne.insert(ComponentType<T>());
  }

  virtual void Start() {}
  virtual void Update(float dt) {}
  virtual void Destroy() {}

protected:
  EntitySignature signature;
  EntitySignature signatureOne;
  std::set<EntityID> entities;
};

}; // namespace aEngine