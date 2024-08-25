#include "Component/MeshRenderer.hpp"

namespace aEngine {

MeshRenderer::MeshRenderer(aEngine::Render::Mesh *mesh) : meshData(mesh) {
  vao.Bind();
  meshData->vbo.BindAs(GL_ARRAY_BUFFER);
  meshData->ebo.BindAs(GL_ELEMENT_ARRAY_BUFFER);
  vao.LinkAttrib(meshData->vbo, 0, 4, GL_FLOAT, sizeof(Vertex), (void *)0);
  vao.LinkAttrib(meshData->vbo, 1, 4, GL_FLOAT, sizeof(Vertex),
                 (void *)(offsetof(Vertex, Normal)));
  vao.LinkAttrib(meshData->vbo, 2, 4, GL_FLOAT, sizeof(Vertex),
                 (void *)(offsetof(Vertex, TexCoords)));
  vao.Unbind();
  meshData->vbo.UnbindAs(GL_ARRAY_BUFFER);
  meshData->ebo.UnbindAs(GL_ARRAY_BUFFER);
}

MeshRenderer::~MeshRenderer() {
  std::cout << "mesh renderer descontruct" << std::endl;
}

void MeshRenderer::DrawMesh(Render::Shader &shader) {
  // draw mesh
  shader.Use();
  vao.Bind();
  if (targetVBO != nullptr) {
    // Bind the duplicated VBO to the same target (GL_ARRAY_BUFFER)
    targetVBO->BindAs(GL_ARRAY_BUFFER);
    // Update the vertex attribute pointers to use the duplicated VBO
    vao.LinkAttrib(*targetVBO, 0, 4, GL_FLOAT, sizeof(Vertex), (void *)0);
    vao.LinkAttrib(*targetVBO, 1, 4, GL_FLOAT, sizeof(Vertex),
                   (void *)(offsetof(Vertex, Normal)));
    vao.LinkAttrib(*targetVBO, 2, 4, GL_FLOAT, sizeof(Vertex),
                   (void *)(offsetof(Vertex, TexCoords)));
  }
  glDrawElements(GL_TRIANGLES,
                 static_cast<unsigned int>(meshData->indices.size()),
                 GL_UNSIGNED_INT, 0);
  vao.Unbind();
  if (targetVBO != nullptr) {
    targetVBO->UnbindAs(GL_ARRAY_BUFFER);
  }
}

void MeshRenderer::ForwardRender(glm::mat4 projMat, glm::mat4 viewMat,
                                 Entity *camera, Entity *object,
                                 std::vector<Light> &lights) {
  for (auto pass : passes) {
    pass->GetShader()->Use();
    pass->SetupLights(GWORLD.Context.activeLights);
    auto modelMat = glm::mat4(1.0f);
    if (targetVBO == nullptr) {
      modelMat = object->GetModelMatrix();
    }
    pass->SetVariables(modelMat, viewMat, projMat, -camera->LocalForward);
    DrawMesh(*pass->GetShader());
    // reset target vbo in case deformerrenderer discard it
    targetVBO = nullptr;
  }
}

void MeshRenderer::DrawInspectorGUI() {
  if (ImGui::TreeNode("MeshRenderer")) {
    ImGui::MenuItem("Options", nullptr, nullptr, false);
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

}; // namespace aEngine