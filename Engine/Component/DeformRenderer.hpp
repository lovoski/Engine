#pragma once

#include "Base/BaseComponent.hpp"
#include "Function/Render/Buffers.hpp"
#include "Function/Render/Mesh.hpp"

#include "Component/Animator.hpp"
#include "Component/MeshRenderer.hpp"
#include "Component/Mesh.hpp"

namespace aEngine {

struct DeformRenderer : public aEngine::BaseComponent {
  DeformRenderer(EntityID id, Animator *ar);
  ~DeformRenderer();

  Animator *animator;
  std::shared_ptr<MeshRenderer> renderer;
  Render::Buffer skeletonMatrices;

  // Deform the mesh with skeleton or shape keys, the meshInstance of
  // `mesh` can't be null.
  void DeformMesh(std::shared_ptr<Mesh> mesh);

  void DrawInspectorGUI() override;
};

}; // namespace aEngine