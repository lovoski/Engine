#include "Component/DeformRenderer.hpp"
#include "Function/Animation/Deform.hpp"

namespace aEngine {

DeformRenderer::DeformRenderer(Render::Mesh *mesh, Animator *ar)
    : animator(ar) {
  renderer = std::make_shared<MeshRenderer>(mesh);
  targetVBO.SetDataAs(GL_SHADER_STORAGE_BUFFER, mesh->vertices);
  skeletonMatrices.SetDataAs(GL_SHADER_STORAGE_BUFFER,
                             ar->GetSkeletonTransforms());
}

DeformRenderer::~DeformRenderer() {
  LOG_F(1, "deconstruct deform renderer");
}

void DeformRenderer::DeformMesh() {
  // setup targetVBO of the renderer
  DeformSkinnedMesh(renderer->meshData, animator, targetVBO, skeletonMatrices);
  renderer->targetVBO = &targetVBO;
}

void DeformRenderer::DrawInspectorGUI() {
  if (ImGui::TreeNode("DeformRenderer")) {
    // ImGui::MenuItem("", nullptr, nullptr, false);
    renderer->DrawInspectorGUI();
    ImGui::TreePop();
  }
}

}; // namespace aEngine