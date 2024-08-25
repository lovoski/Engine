#pragma once

#include "Base/BaseComponent.hpp"
#include "Function/Render/Buffers.hpp"
#include "Function/Render/Mesh.hpp"

#include "Component/Animator.hpp"
#include "Component/MeshRenderer.hpp"

namespace aEngine {

struct DeformRenderer : public aEngine::BaseComponent {
  DeformRenderer(MeshRenderer mr, Animator *ar);
  ~DeformRenderer();

  Animator *animator;
  MeshRenderer renderer;
  Render::Buffer targetVBO, skeletonMatrices;

  void Render(glm::mat4 projMat, glm::mat4 viewMat, Entity *camera,
              Entity *object, std::vector<Light> &lights);

  void DrawInspectorGUI() override;
};

}; // namespace aEngine