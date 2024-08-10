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

// which kind of component this list will stores
template <typename T> class ComponentList : public IComponentList {
public:
  ComponentList() = default;
  ~ComponentList() = default;

  // insert one component to the list if it has not been added
  void Insert(const T &component) {
    auto comp = std::find_if(data.begin(), data.end(), [&](const T &c) {
      return c.GetID() == component.GetID();
    });
    if (comp == data.end()) {
      data.push_back(component);
    }
  }

  T &Get(const EntityID entity) {
    auto comp = std::find_if(data.begin(), data.end(),
                             [&](const T &c) { return c.GetID() == entity; });
    // if (comp == data.end())
    //   throw std::runtime_error("Trying to get non-existing component");
    return *comp;
  }

  void Erase(const EntityID entity) override final {
    auto comp = std::find_if(data.begin(), data.end(),
                             [&](const T &c) { return c.GetID() == entity; });
    if (comp != data.end()) {
      data.erase(comp);
    }
  }

  std::vector<T> data;
};

}; // namespace aEngine