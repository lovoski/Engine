#include "Component/MeshRenderer.hpp"
#include "Utils/AssetsLoader.hpp"
#include "Utils/Render/Mesh.hpp"
#include "Utils/Render/MaterialData.hpp"

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
                                 Material *material,
                                 std::vector<Light> &lights) {
  // each materail could have multiple render passes in different render queues
  for (auto pass : material->passes) {
    auto shader = pass->GetShader();
    shader->Use();
    glm::mat4 ModelToWorldPoint = object->GetModelMatrix();
    glm::mat3 ModelToWorldDir =
        glm::transpose(glm::inverse(glm::mat3(ModelToWorldPoint)));
    shader->SetVec3("ViewDir", -camera->LocalForward);
    shader->SetMat4("Projection", projMat);
    shader->SetMat4("View", viewMat);
    shader->SetMat4("ModelToWorldPoint", ModelToWorldPoint);
    shader->SetMat3("ModelToWorldDir", ModelToWorldDir);
    pass->SetupVariables();
    pass->SetupLights(lights);
    meshData->Draw(*shader);
  }
}

}; // namespace aEngine