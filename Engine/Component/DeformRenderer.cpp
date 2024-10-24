#include "Component/DeformRenderer.hpp"
#include "Component/Mesh.hpp"
#include "Function/Animation/Deform.hpp"

namespace aEngine {

DeformRenderer::DeformRenderer(EntityID id, EntityID anim)
    : animator(anim), BaseComponent(id) {
  renderer = std::make_shared<MeshRenderer>(0);
  skeletonMatrices.SetDataAs(
      GL_SHADER_STORAGE_BUFFER,
      GWORLD.GetComponent<Animator>(animator)->GetSkeletonTransforms());
  FillBlendShapeDataBuffer();
}

DeformRenderer::~DeformRenderer() {}

void DeformRenderer::FillBlendShapeDataBuffer() {
  struct blendshapedata {
    glm::vec4 posOffset[MAX_BLEND_SHAPES];
    glm::vec4 normalOffset[MAX_BLEND_SHAPES];
  };
  if (auto meshInstance =
          GWORLD.GetComponent<Mesh>(entityID)->GetMeshInstance()) {
    std::vector<blendshapedata> data(meshInstance->vertices.size());
    for (int i = 0; i < meshInstance->vertices.size(); ++i) {
      for (int j = 0; j < meshInstance->blendShapes.size(); ++j) {
        data[i].posOffset[j] = glm::vec4(
            meshInstance->blendShapes[j].data[i].BlendShapeOffset, 0.0f);
        data[i].normalOffset[j] = glm::vec4(
            meshInstance->blendShapes[j].data[i].BlendShapeNormal, 0.0f);
      }
    }
    blendShapeDataBuffer.SetDataAs(GL_SHADER_STORAGE_BUFFER, data);
  } else {
    LOG_F(ERROR, "please set <Mesh> component before <DeformRenderer> "
                 "component, blendShapeDataBuffer not setup");
  }
}

void DeformRenderer::DeformMesh(std::shared_ptr<Mesh> mesh) {
  // setup targetVBO of the renderer
  if (animator != 0) {
    auto meshInstace = mesh->GetMeshInstance();
    if (enableBlendShape) {
      std::vector<float> weights;
      for (auto &bs : meshInstace->blendShapes)
        weights.push_back(bs.weight);
      blendShapeWeightsBuffer.SetDataAs(GL_SHADER_STORAGE_BUFFER, weights);
      DeformBlendSkinnedMesh(GWORLD.GetComponent<Animator>(animator).get(),
                             meshInstace->vbo, meshInstace->vertices.size(),
                             mesh->target, skeletonMatrices,
                             meshInstace->blendShapes.size(),
                             blendShapeWeightsBuffer, blendShapeDataBuffer);
    } else {
      DeformSkinnedMesh(GWORLD.GetComponent<Animator>(animator).get(),
                        meshInstace->vbo, meshInstace->vertices.size(),
                        mesh->target, skeletonMatrices);
    }
    mesh->Deformed = true; // setup the flag
  }
}

void DeformRenderer::DrawInspectorGUI() {
  if (ImGui::TreeNode("Blend Shapes")) {
    auto mesh = GWORLD.GetComponent<Mesh>(entityID)->GetMeshInstance();
    ImGui::Checkbox("Enable Blend Shapes", &enableBlendShape);
    if (!enableBlendShape)
      ImGui::BeginDisabled();
    for (auto &bs : mesh->blendShapes)
      ImGui::DragFloat(bs.name.c_str(), &bs.weight, 0.002f, 0.0f, 1.0f);
    if (!enableBlendShape)
      ImGui::EndDisabled();
    ImGui::TreePop();
  }

  if (ImGui::TreeNode("Renderer")) {
    renderer->DrawInspectorGUI();
    ImGui::TreePop();
  }
}

}; // namespace aEngine

REGISTER_COMPONENT(aEngine, DeformRenderer);