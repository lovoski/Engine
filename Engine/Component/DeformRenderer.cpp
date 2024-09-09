#include "Component/DeformRenderer.hpp"
#include "Function/Animation/Deform.hpp"

namespace aEngine {

DeformRenderer::DeformRenderer(EntityID id, Animator *ar)
    : animator(ar), BaseComponent(id) {
  renderer = std::make_shared<MeshRenderer>(id);
  skeletonMatrices.SetDataAs(GL_SHADER_STORAGE_BUFFER,
                             ar->GetSkeletonTransforms());
}

DeformRenderer::~DeformRenderer() { LOG_F(1, "deconstruct deform renderer"); }

void DeformRenderer::DeformMesh(std::shared_ptr<Mesh> mesh) {
  // setup targetVBO of the renderer
  DeformSkinnedMesh(animator, mesh->meshInstance->vbo,
                    mesh->meshInstance->vertices.size(), mesh->target,
                    skeletonMatrices);
  mesh->Deformed = true; // setup the flag
}

void DeformRenderer::DrawInspectorGUI() {
  // ImGui::MenuItem("", nullptr, nullptr, false);
  if (ImGui::TreeNode("Renderer")) {
    renderer->DrawInspectorGUI();
    ImGui::TreePop();
  }
}

}; // namespace aEngine