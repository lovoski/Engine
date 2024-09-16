#include "Component/MeshRenderer.hpp"

#include "Function/Render/RenderPass.hpp"

namespace aEngine {

MeshRenderer::MeshRenderer(EntityID id) : BaseComponent(id) {}

MeshRenderer::~MeshRenderer() {}

void MeshRenderer::DrawMesh(Render::Shader &shader,
                            std::shared_ptr<Mesh> mesh) {
  shader.Use();
  mesh->Bind();
  glDrawElements(
      GL_TRIANGLES,
      static_cast<unsigned int>(mesh->GetMeshInstance()->indices.size()),
      GL_UNSIGNED_INT, 0);
  mesh->Unbind();
}

void MeshRenderer::DrawMeshShadowPass(Render::Shader &shader,
                                      std::shared_ptr<Mesh> mesh,
                                      glm::mat4 modelMat) {
  shader.Use();
  auto model = glm::mat4(1.0f);
  if (!mesh->Deformed)
    model = modelMat;
  shader.SetMat4("Model", model);
  DrawMesh(shader, mesh);
  mesh->Deformed = false;
}

void MeshRenderer::ForwardRender(std::shared_ptr<Mesh> mesh, glm::mat4 projMat,
                                 glm::mat4 viewMat, Entity *camera,
                                 Entity *object, Render::Buffer &lightsBuffer) {
  for (auto pass : passes) {
    if (pass->Enabled) {
      pass->GetShader()->Use();
      pass->SetupLights(lightsBuffer);
      auto modelMat = glm::mat4(1.0f);
      if (!mesh->Deformed)
        modelMat = object->GlobalTransformMatrix();
      auto viewDir = -camera->LocalForward;
      pass->BeforePassInternal(modelMat, viewMat, projMat, viewDir,
                      receiveShadow);
      DrawMesh(*pass->GetShader(), mesh);
      pass->FinishPass();
    }
  }
  // // reset mesh deform state
  // mesh->Deformed = false;
}

void MeshRenderer::drawAppendPassPopup() {
  if (ImGui::BeginPopup("appendpasspanelpopup")) {
    ImGui::MenuItem("Registered Pass", nullptr, nullptr, false);
    ImGui::Separator();
    if (ImGui::MenuItem("Basic Pass"))
      handleAppendPass<Render::Basic>("Basic");
    ImGui::Separator();
    if (ImGui::MenuItem("Wireframe Pass"))
      handleAppendPass<Render::WireFramePass>("Wireframe");
    if (ImGui::MenuItem("Outline Pass"))
      handleAppendPass<Render::OutlinePass>("Outline");
    if (ImGui::MenuItem("GBV Toon Pass"))
      handleAppendPass<Render::GBVMainPass>("GBV Main");
    ImGui::Separator();
    if (ImGui::MenuItem("PBR Pass"))
      handleAppendPass<Render::PBRPass>("PBR");
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
  for (auto pass : passes) {
    ImGui::Separator();
    if (ImGui::TreeNode(pass->getInspectorWindowName().c_str())) {
      pass->DrawInspectorGUIInternal();
      ImGui::TreePop();
    }
  }
}

}; // namespace aEngine