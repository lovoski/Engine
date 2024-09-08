#include "Component/MeshRenderer.hpp"

#include "Function/Render/RenderPass.hpp"

namespace aEngine {

MeshRenderer::MeshRenderer(EntityID id, aEngine::Render::Mesh *mesh)
    : meshData(mesh), BaseComponent(id) {
  vao.Bind();
  meshData->vbo.BindAs(GL_ARRAY_BUFFER);
  meshData->ebo.BindAs(GL_ELEMENT_ARRAY_BUFFER);
  vao.LinkAttrib(meshData->vbo, 0, 4, GL_FLOAT, sizeof(Vertex), (void *)0);
  vao.LinkAttrib(meshData->vbo, 1, 4, GL_FLOAT, sizeof(Vertex),
                 (void *)(offsetof(Vertex, Normal)));
  vao.LinkAttrib(meshData->vbo, 2, 4, GL_FLOAT, sizeof(Vertex),
                 (void *)(offsetof(Vertex, TexCoords)));
  vao.LinkAttrib(meshData->vbo, 3, 4, GL_FLOAT, sizeof(Vertex),
                 (void *)(offsetof(Vertex, Color))); // vertex color
  vao.Unbind();
  meshData->vbo.UnbindAs(GL_ARRAY_BUFFER);
  meshData->ebo.UnbindAs(GL_ARRAY_BUFFER);
}

MeshRenderer::~MeshRenderer() { LOG_F(1, "deconstruct mesh renderer"); }

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
    vao.LinkAttrib(*targetVBO, 3, 4, GL_FLOAT, sizeof(Vertex),
                   (void *)(offsetof(Vertex, Color))); // vertex color
  }
  glDrawElements(GL_TRIANGLES,
                 static_cast<unsigned int>(meshData->indices.size()),
                 GL_UNSIGNED_INT, 0);
  vao.Unbind();
  if (targetVBO != nullptr) {
    targetVBO->UnbindAs(GL_ARRAY_BUFFER);
  }
}

void MeshRenderer::DrawMeshShadowPass(Render::Shader &shader,
                                      glm::mat4 modelMat) {
  shader.Use();
  auto model = glm::mat4(1.0f);
  if (targetVBO == nullptr) {
    model = modelMat;
  }
  shader.SetMat4("Model", model);
  DrawMesh(shader);
  targetVBO = nullptr;
}

void MeshRenderer::ForwardRender(glm::mat4 projMat, glm::mat4 viewMat,
                                 Entity *camera, Entity *object,
                                 Render::Buffer &lightsBuffer) {
  for (auto pass : passes) {
    if (pass->Enabled) {
      pass->GetShader()->Use();
      pass->SetupLights(lightsBuffer);
      auto modelMat = glm::mat4(1.0f);
      if (targetVBO == nullptr) {
        modelMat = object->GetModelMatrix();
      }
      pass->SetupPass(modelMat, viewMat, projMat, -camera->LocalForward,
                      receiveShadow);
      DrawMesh(*pass->GetShader());
      pass->FinishPass();
    }
  }
  // reset target vbo in case deformerrenderer discard it
  targetVBO = nullptr;
  // reset lightSpaceMatrices after all passes
  lightSpaceMatrices.clear();
}

void MeshRenderer::drawAppendPassPopup() {
  if (ImGui::BeginPopup("appendpasspanelpopup")) {
    ImGui::MenuItem("Registered Pass", nullptr, nullptr, false);
    ImGui::Separator();
    if (ImGui::MenuItem("Outline Pass"))
      AddPass<Render::OutlinePass>(nullptr, "Outline");
    if (ImGui::MenuItem("Diffuse Pass"))
      AddPass<Render::Diffuse>(nullptr, "Diffuse");
    if (ImGui::MenuItem("GBV Toon Pass"))
      AddPass<Render::GBVMainPass>(nullptr, "GBV Main");
    ImGui::EndPopup();
  }
}

void MeshRenderer::DrawInspectorGUI() {
  ImGui::MenuItem("Options", nullptr, nullptr, false);
  ImGui::Checkbox("Cast Shadow", &castShadow);
  ImGui::Checkbox("Receive Shadow", &receiveShadow);
  ImGui::MenuItem("Render Passes", nullptr, nullptr, false);
  if (ImGui::Button("Append Pass", {-1, 30}))
    ImGui::OpenPopup("appendpasspanelpopup");
  drawAppendPassPopup();
  Render::BasePass *passToRemove = nullptr;
  for (auto pass : passes) {
    ImGui::Separator();
    if (ImGui::TreeNode(pass->GetMaterialTypeName().c_str())) {
      pass->DrawInspectorGUI();
      if (ImGui::IsWindowHovered() &&
          ImGui::IsMouseClicked(ImGuiMouseButton_Right))
        ImGui::OpenPopup("rightclickpass");
      if (ImGui::BeginPopup("rightclickpass")) {
        ImGui::MenuItem("Options", nullptr, nullptr, false);
        if (ImGui::MenuItem("Reset")) {
        }
        if (ImGui::MenuItem("Remove")) {
          passToRemove = pass;
          LOG_F(INFO, "remove pass %s from entity %d",
                pass->GetMaterialTypeName().c_str(), entityID);
        }
        ImGui::EndPopup();
      }
      ImGui::TreePop();
    }
  }
  if (passToRemove) {
    for (auto it = passes.begin(); it != passes.end(); ++it)
      if ((*it) == passToRemove) {
        passes.erase(it);
        break;
      }
  }
}

}; // namespace aEngine