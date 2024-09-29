#include "Component/DeformRenderer.hpp"
#include "Function/Animation/Deform.hpp"

namespace aEngine {

DeformRenderer::DeformRenderer(EntityID id, Animator *ar)
    : animator(ar), BaseComponent(id) {
  renderer = std::make_shared<MeshRenderer>(id);
  skeletonMatrices.SetDataAs(GL_SHADER_STORAGE_BUFFER,
                             ar->GetSkeletonTransforms());
}

DeformRenderer::~DeformRenderer() {}

void DeformRenderer::DeformMesh(std::shared_ptr<Mesh> mesh) {
  // setup targetVBO of the renderer
  if (animator) {
    DeformSkinnedMesh(animator, mesh->GetMeshInstance()->vbo,
                      mesh->GetMeshInstance()->vertices.size(), mesh->target,
                      skeletonMatrices);
    mesh->Deformed = true; // setup the flag
  }
}

void DeformRenderer::DrawInspectorGUI() {
  // ImGui::MenuItem("", nullptr, nullptr, false);
  if (ImGui::TreeNode("Renderer")) {
    renderer->DrawInspectorGUI();
    ImGui::TreePop();
  }
}

}; // namespace aEngine

REGISTER_COMPONENT(aEngine, DeformRenderer)