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
  std::cout << "deconstruct deform renderer" << std::endl;
  targetVBO.Delete();
  skeletonMatrices.Delete();
}

void DeformRenderer::Render(glm::mat4 projMat, glm::mat4 viewMat,
                            Entity *camera, Entity *object,
                            std::vector<std::shared_ptr<Light>> &lights) {
  // setup targetVBO of the renderer
  DeformSkinnedMesh(renderer->meshData, animator, targetVBO, skeletonMatrices);
  renderer->targetVBO = &targetVBO;
  // render as if it were a normal mesh
  renderer->ForwardRender(projMat, viewMat, camera, object, lights);
}

void DeformRenderer::DrawInspectorGUI() {
  if (ImGui::TreeNode("DeformRenderer")) {
    ImGui::MenuItem("Options", nullptr, nullptr, false);
    if (ImGui::TreeNode("Render Passes")) {
      for (auto pass : renderer->passes) {
        ImGui::Separator();
        pass->DrawInspectorGUI();
      }
      ImGui::TreePop();
    }
    ImGui::TreePop();
  }
}

}; // namespace aEngine