// Stores all the components in a compact array
#pragma once

#include "Base/Types.hpp"

namespace aEngine {

// make it possible to store all component lists together
class IComponentList {
public:
  IComponentList() = default;
  virtual ~IComponentList() = default;
  virtual void Erase(const EntityID entity) {}
};

// The component list will hold shared pointers of type T
template <typename T> class ComponentList : public IComponentList {
public:
  ComponentList() = default;
  ~ComponentList() = default;

  // insert one component to the list if it has not been added
  void Insert(const std::shared_ptr<T> component) {
    auto comp = std::find_if(data.begin(), data.end(), [&](const std::shared_ptr<T> c) {
      return c->GetID() == component->GetID();
    });
    if (comp == data.end()) {
      data.push_back(component);
    }
  }

  std::shared_ptr<T> Get(const EntityID entity) {
    auto comp = std::find_if(data.begin(), data.end(),
                             [&](const std::shared_ptr<T> c) { return c->GetID() == entity; });
    if (comp == data.end()) {
      LOG_F(WARNING, "Get non-existing component from entity %d", entity);
      return nullptr;
    }
    return (*comp);
  }

  void Erase(const EntityID entity) override final {
    auto comp = std::find_if(data.begin(), data.end(),
                             [&](const std::shared_ptr<T> c) { return c->GetID() == entity; });
    if (comp != data.end()) {
      data.erase(comp);
    }
  }

  std::vector<std::shared_ptr<T>> data;
};

}; // namespace aEngine