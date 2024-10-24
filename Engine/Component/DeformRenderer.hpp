#pragma once

#include "Base/BaseComponent.hpp"
#include "Function/Render/Buffers.hpp"
#include "Function/Render/Mesh.hpp"

#include "Component/Animator.hpp"
#include "Component/Mesh.hpp"
#include "Component/MeshRenderer.hpp"

namespace aEngine {

struct DeformRenderer : public aEngine::BaseComponent {
  DeformRenderer() : BaseComponent(0) {
    animator = 0;
    renderer = nullptr;
  }
  DeformRenderer(EntityID id, EntityID anim);
  ~DeformRenderer();

  EntityID animator;
  std::shared_ptr<MeshRenderer> renderer;
  Render::Buffer skeletonMatrices;
  // This buffer gets updated per frame
  Render::Buffer blendShapeWeightsBuffer;
  // This buffer only gets updated when blend shape itself changes
  Render::Buffer blendShapeDataBuffer;

  std::string getInspectorWindowName() override { return "Deform Renderer"; }

  template <typename Archive> void save(Archive &ar) const {
    ar(CEREAL_NVP(entityID), animator, renderer);
  }
  template <typename Archive> void load(Archive &ar) {
    ar(CEREAL_NVP(entityID), animator, renderer);
  }

  void DeformMesh(std::shared_ptr<Mesh> mesh);

  void FillBlendShapeDataBuffer();

  void DrawInspectorGUI() override;

private:
  bool enableBlendShape = false;
};

}; // namespace aEngine