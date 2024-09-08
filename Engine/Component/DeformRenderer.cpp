#include "Component/DeformRenderer.hpp"
#include "Function/Animation/Deform.hpp"

namespace aEngine {

DeformRenderer::DeformRenderer(EntityID id, Render::Mesh *mesh, Animator *ar)
    : animator(ar), BaseComponent(id) {
  renderer = std::make_shared<MeshRenderer>(id, mesh);
  targetVBO.SetDataAs(GL_SHADER_STORAGE_BUFFER, mesh->vertices);
  skeletonMatrices.SetDataAs(GL_SHADER_STORAGE_BUFFER,
                             ar->GetSkeletonTransforms());
}

DeformRenderer::~DeformRenderer() { LOG_F(1, "deconstruct deform renderer"); }

void DeformRenderer::DeformMesh() {
  // setup targetVBO of the renderer
  DeformSkinnedMesh(animator, renderer->meshData->vbo,
                    renderer->meshData->vertices.size(), targetVBO,
                    skeletonMatrices);
  renderer->targetVBO = &targetVBO;
}

void DeformRenderer::DrawInspectorGUI() {
  // ImGui::MenuItem("", nullptr, nullptr, false);
  if (ImGui::TreeNode("Renderer")) {
    renderer->DrawInspectorGUI();
    ImGui::TreePop();
  }
}

}; // namespace aEngine