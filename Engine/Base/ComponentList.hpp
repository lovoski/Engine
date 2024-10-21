// Stores all the components in a compact array
#pragma once

#include "Base/BaseSystem.hpp"
#include "Base/Types.hpp"

#include <cereal/types/base_class.hpp>

namespace aEngine {

// make it possible to store all component lists together
class IComponentList {
public:
  IComponentList() = default;
  virtual ~IComponentList() = default;
  virtual void Erase(const EntityID entity) {}

  // Returns true if the entity has this component, false otherwise
  virtual bool DrawInspectorGUI(const EntityID entity) { return false; }

  // Returns true if the entity has this component
  virtual bool Has(const EntityID entity) { return false; }

  virtual void Clear() {}

  virtual std::string getInspectorWindowName() { return ""; }

  template <typename Archive> void serialize(Archive &ar) {}
};

// The component list will hold shared pointers of type T
template <typename T> class ComponentList : public IComponentList {
public:
  ComponentList() = default;
  ~ComponentList() = default;

  // insert one component to the list if it has not been added
  void Insert(const std::shared_ptr<T> component) {
    auto comp =
        std::find_if(data.begin(), data.end(), [&](const std::shared_ptr<T> c) {
          return c->GetID() == component->GetID();
        });
    if (comp == data.end()) {
      data.push_back(component);
    }
  }

  std::shared_ptr<T> Get(const EntityID entity) {
    auto comp =
        std::find_if(data.begin(), data.end(), [&](const std::shared_ptr<T> c) {
          return c->GetID() == entity;
        });
    if (comp == data.end()) {
      // LOG_F(WARNING, "Get non-existing component %s from entity %d",
      // typeid(T).name(), entity);
      return nullptr;
    }
    return (*comp);
  }

  void Erase(const EntityID entity) override {
    auto comp =
        std::find_if(data.begin(), data.end(), [&](const std::shared_ptr<T> c) {
          return c->GetID() == entity;
        });
    if (comp != data.end()) {
      data.erase(comp);
    }
  }

  bool Has(const EntityID entity) override {
    auto comp =
        std::find_if(data.begin(), data.end(), [&](const std::shared_ptr<T> c) {
          return c->GetID() == entity;
        });
    return comp != data.end();
  }

  bool DrawInspectorGUI(const EntityID entity) override {
    auto comp =
        std::find_if(data.begin(), data.end(), [&](const std::shared_ptr<T> c) {
          return c->GetID() == entity;
        });
    if (comp != data.end()) {
      (*comp)->DrawInspectorGUI();
      return true;
    } else
      return false;
  }

  void Clear() override { data.clear(); }

  std::string getInspectorWindowName() override {
    auto it = BaseSystem::CompMap.find(ComponentType<T>());
    return (*it).second->getInspectorWindowName();
  }

  std::vector<std::shared_ptr<T>> data;

  template <typename Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<IComponentList>(this), data);
  }
};

}; // namespace aEngine

#define REGISTER_COMPONENT(Namespace, ComponentType)                           \
  CEREAL_REGISTER_TYPE(aEngine::ComponentList<Namespace::ComponentType>);      \
  CEREAL_REGISTER_POLYMORPHIC_RELATION(                                        \
      aEngine::IComponentList,                                                 \
      aEngine::ComponentList<Namespace::ComponentType>)