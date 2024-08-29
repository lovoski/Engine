#pragma once

#include "Base/BaseComponent.hpp"
#include "Function/Render/Buffers.hpp"
#include "Function/Render/Mesh.hpp"

#include "Component/Animator.hpp"
#include "Component/MeshRenderer.hpp"

namespace aEngine {

struct DeformRenderer : public aEngine::BaseComponent {
  DeformRenderer(Render::Mesh *mesh, Animator *ar);
  ~DeformRenderer();

  Animator *animator;
  std::shared_ptr<MeshRenderer> renderer;
  Render::Buffer targetVBO, skeletonMatrices;

  // Add a render pass for this renderer, pass in the nullptr
  // to instantiate a new pass of the defualt type
  template <typename T> void AddPass(T *pass, std::string identifier) {
    renderer->AddPass<T>(pass, identifier);
  }

  void Render(glm::mat4 projMat, glm::mat4 viewMat, Entity *camera,
              Entity *object, std::vector<std::shared_ptr<Light>> &lights);

  void DrawInspectorGUI() override;
};

}; // namespace aEngine