#pragma once

#include "Base/BaseComponent.hpp"
#include "Function/Render/Buffers.hpp"
#include "Function/Render/Mesh.hpp"

#include "Component/Animator.hpp"
#include "Component/Mesh.hpp"
#include "Component/MeshRenderer.hpp"

namespace aEngine {

struct DeformRenderer : public aEngine::BaseComponent {
  DeformRenderer() : BaseComponent(-1) {
    animator = nullptr;
    renderer = nullptr;
  }
  DeformRenderer(EntityID id, Animator *ar);
  ~DeformRenderer();

  Animator *animator;
  std::shared_ptr<MeshRenderer> renderer;
  Render::Buffer skeletonMatrices;

  void DeformMesh(std::shared_ptr<Mesh> mesh);

  void DrawInspectorGUI() override;
};

}; // namespace aEngine