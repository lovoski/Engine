#include "Component/Animator.hpp"
#include "Component/MeshRenderer.hpp"
#include "Function/AssetsLoader.hpp"
#include "Function/AssetsType.hpp"
#include "Function/Render/MaterialData.hpp"
#include "Function/Render/Mesh.hpp"
#include "Function/Render/Shader.hpp"
#include "Function/Render/VisUtils.hpp"

namespace aEngine {

Json MeshRenderer::Serialize() {
  Json json;
  json["mesh"]["modelpath"] = meshData->modelPath;
  json["mesh"]["identifier"] = meshData->identifier;
  return json;
}

void MeshRenderer::Deserialize(Json &json) {
  std::string modelPath = json["mesh"]["modelpath"];
  std::string identifier = json["mesh"]["identifier"];
  meshData = Loader.GetMesh(modelPath, identifier);
}

void MeshRenderer::ForwardRender(glm::mat4 projMat, glm::mat4 viewMat,
                                 Entity *camera, Entity *object,
                                 std::vector<Light> &lights) {
  // apply all the deformers before actual rendering
  for (auto deformer : deformers) {
    deformer->DeformMesh(object, true);
  }
  for (auto pass : passes) {
    pass->GetShader()->Use();
    pass->SetupLights(GWORLD.Context.activeLights);
    pass->SetVariables(object->GetModelMatrix(), viewMat, projMat,
                       -camera->LocalForward);
    meshData->Draw(*pass->GetShader());
  }
}

void MeshRenderer::DrawInspectorGUI() {
  if (ImGui::TreeNode("MeshRenderer")) {
    ImGui::MenuItem("Options", nullptr, nullptr, false);
    if (ImGui::TreeNode("Deformers")) {
      for (auto deformer : deformers) {
        ImGui::Separator();
        deformer->DrawInspectorGUI();
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Render Passes")) {
      for (auto pass : passes) {
        ImGui::Separator();
        pass->DrawInspectorGUI();
      }
      ImGui::TreePop();
    }
    ImGui::TreePop();
  }
}

std::vector<glm::mat4> Animator::GetSkeletonTransforms() {
  std::vector<glm::mat4> result(CurrentPose.skeleton->GetNumJoints(),
                                glm::mat4(1.0f));
  // capture position, rotation of joints, convert these information into matrices

  return result;
}

}; // namespace aEngine