#pragma once

#include "Base/ComponentList.hpp"
#include "Entity.hpp"
#include "Scene.hpp"

namespace aEngine {

class CubeCollider : public BaseComponent {
public:
  CubeCollider() : BaseComponent(0) {}
  CubeCollider(EntityID id);
  ~CubeCollider();

private:
};

}; // namespace aEngine