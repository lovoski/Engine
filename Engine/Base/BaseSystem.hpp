#pragma once

#include "Base/BaseComponent.hpp"
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
    CompMap.insert(std::make_pair(ComponentType<T>(), std::make_unique<T>()));
  }

  // Add some component to a system, the entity must has at least one of the
  // component registered with this function
  template <typename T> void AddComponentSignatureRequireOne() {
    signatureOne.insert(ComponentType<T>());
    CompMap.insert(std::make_pair(ComponentType<T>(), std::make_unique<T>()));
  }

  // Initialize system related resources
  virtual void Start() {}
  // This function should only be overloaded to update some readonly
  // variables before update function to avoid over compute
  virtual void PreUpdate(float dt) {}
  virtual void Update(float dt) {}
  // Destroy system related resources
  virtual void Destroy() {}
  // Reset local variables of the system.
  virtual void Reset() {}

  const int GetNumEntities() const { return entities.size(); }

  // Get all the ids of entities maintained by this system.
  const std::set<EntityID> GetEntities() const { return entities; }

  // from component id to component instance
  static std::map<ComponentTypeID, std::unique_ptr<BaseComponent>> CompMap;

  template <typename Archive>
  void serialize(Archive &archive, const unsigned int version) {
    archive &entities;
    archive &signature;
    archive &signatureOne;
  }

protected:
  EntitySignature signature;
  EntitySignature signatureOne;
  std::set<EntityID> entities;
};

}; // namespace aEngine