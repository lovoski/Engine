#pragma once

#include "Base/ComponentList.hpp"
#include "Entity.hpp"
#include "Scene.hpp"

namespace aEngine {

class RigidBody : public BaseComponent {
public:
  RigidBody() : BaseComponent(0) {}
  RigidBody(EntityID id);
  ~RigidBody();

  template <typename Archive> void serialize(Archive &ar) {}

private:
};

}; // namespace aEngine