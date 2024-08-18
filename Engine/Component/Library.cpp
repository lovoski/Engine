#include "Component/MeshRenderer.hpp"
#include "Utils/AssetsLoader.hpp"
#include "Utils/AssetsType.hpp"
#include "Utils/Render/MaterialData.hpp"
#include "Utils/Render/Mesh.hpp"
#include "Utils/Render/Shader.hpp"
#include "Utils/Render/VisUtils.hpp"

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
    if (ImGui::TreeNode("Passes")) {
      for (auto pass : passes) {
        ImGui::Separator();
        pass->DrawInspectorGUI();
      }
      ImGui::TreePop();
    }
    ImGui::TreePop();
  }
}

}; // namespace aEngine