#include "Function/Render/RenderPass.hpp"
#include "Entity.hpp"
#include "Function/AssetsLoader.hpp"
#include "Scene.hpp"

namespace aEngine {

namespace Render {

using glm::vec2;
using glm::vec3;
using std::string;
using std::vector;

bool ActivateTexture2D(Texture &texture, Shader *shader, string name,
                       int slot) {
  shader->Use(); // activate the shader
  if (texture.path != "::NULL_ICON") {
    // only activate none icon texture
    glActiveTexture(GL_TEXTURE0 + slot);
    int location = glGetUniformLocation(shader->ID, name.c_str());
    if (location == -1)
      return false;
    glUniform1i(location, texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
  }
  return true;
}

void BasePass::SetupLights(Buffer &lightsBuffer, int bindingPoint) {
  if (shader == nullptr)
    LOG_F(ERROR, "shader not setup for render pass %s", identifier.c_str());
  shader->Use();
  lightsBuffer.BindToPointAs(GL_SHADER_STORAGE_BUFFER, bindingPoint);
}

std::string BasePass::getMaterialTypeName() { return typeid(*this).name(); }

void BasePass::drawInspectorGUIDefault() {
  ImGui::MenuItem("Name:", nullptr, nullptr, false);
  ImGui::TextWrapped("%s", identifier.c_str());
  ImGui::MenuItem("Type:", nullptr, nullptr, false);
  ImGui::TextWrapped("%s", getMaterialTypeName().c_str());
  ImGui::MenuItem("Options:", nullptr, nullptr, false);
  ImGui::Checkbox(("Enabled##" + getMaterialTypeName() + identifier).c_str(),
                  &Enabled);
  ImGui::MenuItem("Properties:", nullptr, nullptr, false);
}

void BasePass::SetupPass(glm::mat4 &model, glm::mat4 &view,
                         glm::mat4 &projection, glm::vec3 &viewDir, bool receiveShadow) {
  if (shader != nullptr) {
    shader->Use();
    glm::mat4 ModelToWorldPoint = model;
    glm::mat3 ModelToWorldDir = glm::mat3(ModelToWorldPoint);
    shader->SetMat4("Projection", projection);
    shader->SetMat4("View", view);
    shader->SetMat4("ModelToWorldPoint", ModelToWorldPoint);
    shader->SetMat3("ModelToWorldDir", ModelToWorldDir);
    shader->SetVec3("ViewDir", viewDir);
    shader->SetInt("ReceiveShadow", receiveShadow);
    additionalSetup();
  }
};

// ----------------------Diffuse Material------------------------------

Diffuse::Diffuse() {
  // initialize shader to defualt value
  shader = Loader.GetShader("::diffuse");
}

void Diffuse::DrawInspectorGUI() {
  drawInspectorGUIDefault();
  ImGui::SliderFloat("Ambient", &Ambient, 0.0f, 1.0f);
  float albedoColor[3] = {Albedo.x, Albedo.y, Albedo.z};
  if (ImGui::ColorEdit3("Albedo", albedoColor)) {
    Albedo = glm::vec3(albedoColor[0], albedoColor[1], albedoColor[2]);
  }
}

void Diffuse::additionalSetup() {
  shader->Use();
  shader->SetVec3("Albedo", Albedo);
  shader->SetFloat("Ambient", Ambient);
}

std::string Diffuse::getMaterialTypeName() { return "Diffuse"; }

// ----------------------- Outline -----------------------

OutlinePass::OutlinePass() { shader = Loader.GetShader("::outline"); }

void OutlinePass::DrawInspectorGUI() {
  drawInspectorGUIDefault();
  ImGui::SliderFloat("Width", &OutlineWidth, 0.0f, 1.0f);
  ImGui::SliderFloat("Weight", &OutlineWeight, 0.0f, 1.0f);
  float outlineColor[3] = {OutlineColor.x, OutlineColor.y, OutlineColor.z};
  if (ImGui::ColorEdit3("Color", outlineColor)) {
    OutlineColor = glm::vec3(outlineColor[0], outlineColor[1], outlineColor[2]);
  }
}

void OutlinePass::additionalSetup() {
  shader->Use();
  shader->SetFloat("OutlineWidth", OutlineWidth);
  shader->SetFloat("OutlineWeight", OutlineWeight);
  shader->SetVec3("OutlineColor", OutlineColor);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);
}

void OutlinePass::FinishPass() { glDisable(GL_CULL_FACE); }

std::string OutlinePass::getMaterialTypeName() { return "Outline"; }

}; // namespace Render

}; // namespace aEngine