#pragma once

#include "object/Mesh.hpp"
#include "ecs/ecs.hpp"

class MeshRenderer : public ECS::BaseComponent {
public:
  MeshRenderer() {}
  ~MeshRenderer() {}

  std::shared_ptr<Graphics::Model> Renderer;
};