#include "Function/Render/RenderPass.hpp"
#include "Entity.hpp"
#include "Function/AssetsLoader.hpp"
#include "Function/GUI/Helpers.hpp"
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
  static Texture pureWhite = *Loader.GetTexture("::white_texture");
  auto activateTexId = texture.id;
  if (texture.path == "::null_texture") {
    activateTexId = pureWhite.id;
  }
  glActiveTexture(GL_TEXTURE0 + slot);
  int location = glGetUniformLocation(shader->ID, name.c_str());
  if (location == -1) {
    // LOG_F(WARNING, "location for %s not valid", name.c_str());
    return false;
  }
  glUniform1i(location, slot);
  glBindTexture(GL_TEXTURE_2D, activateTexId);
  return true;
}

void BasePass::SetupLights(Buffer &lightsBuffer, int bindingPoint) {
  if (shader == nullptr)
    LOG_F(ERROR, "shader not setup for render pass %s", identifier.c_str());
  shader->Use();
  lightsBuffer.BindToPointAs(GL_SHADER_STORAGE_BUFFER, bindingPoint);
}

std::string BasePass::GetMaterialTypeName() { return typeid(*this).name(); }

void BasePass::DrawInspectorGUI() {
  ImGui::Checkbox("Enable Pass", &Enabled);
  ImGui::Separator();
  if (!Enabled)
    ImGui::BeginDisabled();
  drawCustomInspectorGUI();
  if (!Enabled)
    ImGui::EndDisabled();
}

void BasePass::SetupPass(glm::mat4 &model, glm::mat4 &view,
                         glm::mat4 &projection, glm::vec3 &viewDir,
                         bool receiveShadow) {
  if (shader != nullptr) {
    shader->Use();
    glm::mat4 ModelToWorldPoint = model;
    glm::mat3 ModelToWorldDir = glm::mat3(ModelToWorldPoint);
    shader->SetMat4("Projection", projection);
    shader->SetMat4("View", view);
    shader->SetMat4("ModelToWorldPoint", ModelToWorldPoint);
    shader->SetMat3("ModelToWorldDir", ModelToWorldDir);
    shader->SetVec3("ViewDir", viewDir);
    shader->SetVec2("ViewportSize", GWORLD.Context.sceneWindowSize);
    shader->SetInt("ReceiveShadow", receiveShadow);
    additionalSetup();
  }
};

// ----------------------Basic Material------------------------------

Basic::Basic() {
  // initialize shader to defualt value
  shader = Loader.GetShader("::diffuse");
}

void Basic::drawCustomInspectorGUI() {
  ImGui::Checkbox("Wireframe", &withWireframe);
  ImGui::SliderFloat("Width", &WireframeWidth, 0.5f, 5.0f);
  GUIUtils::ColorEdit3(WireframeColor, "Color");
  ImGui::Separator();
  ImGui::Checkbox("View Normal", &viewNormal);
  ImGui::Separator();
  GUIUtils::ColorEdit3(Albedo, "Albedo");
}

void Basic::additionalSetup() {
  shader->Use();
  shader->SetVec3("Albedo", Albedo);
  shader->SetFloat("Ambient", Ambient);
  shader->SetVec3("WireframeColor", WireframeColor);
  shader->SetFloat("WireframeWidth", WireframeWidth);
  shader->SetBool("Wireframe", withWireframe);
  shader->SetBool("ViewNormal", viewNormal);
}

std::string Basic::GetMaterialTypeName() { return "Basic"; }

// ----------------------- Outline -----------------------

OutlinePass::OutlinePass() {
  shader = Loader.GetShader("::outline");
  OutlineColorMap = *Loader.GetTexture("::null_texture");
}

void OutlinePass::drawCustomInspectorGUI() {
  ImGui::DragFloat("Width", &OutlineWidth, 0.001f, 0.0f, 10.0f);
  ImGui::SliderFloat("Weight", &OutlineWeight, 0.0f, 1.0f);
  float outlineColor[3] = {OutlineColor.x, OutlineColor.y, OutlineColor.z};
  if (ImGui::ColorEdit3("Color", outlineColor)) {
    OutlineColor = glm::vec3(outlineColor[0], outlineColor[1], outlineColor[2]);
  }
  GUIUtils::DragableTextureTarget(OutlineColorMap, "Outline Map");
}

void OutlinePass::additionalSetup() {
  shader->Use();
  shader->SetFloat("OutlineWidth", OutlineWidth);
  shader->SetFloat("OutlineWeight", OutlineWeight);
  shader->SetVec3("OutlineColor", OutlineColor);
  ActivateTexture2D(OutlineColorMap, shader, "OutlineMap", 0);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);
}

void OutlinePass::FinishPass() { glDisable(GL_CULL_FACE); }

std::string OutlinePass::GetMaterialTypeName() { return "Outline"; }

// ----------- Wireframe Shader -----------

WireFramePass::WireFramePass() { shader = Loader.GetShader("::wireframe"); }

void WireFramePass::FinishPass() {
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

std::string WireFramePass::GetMaterialTypeName() { return "Wireframe"; }

void WireFramePass::additionalSetup() {
  shader->Use();
  shader->SetVec3("wireframeColor", wireFrameColor);
  shader->SetFloat("wireframeOffset", wireframeOffset);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDisable(GL_CULL_FACE);
}

void WireFramePass::drawCustomInspectorGUI() {
  ImGui::DragFloat("Offset", &wireframeOffset, 0.0001, 0.0f, 10.0f);
  GUIUtils::ColorEdit3(wireFrameColor, "Color");
}

// ----------- GBV shader -------------

GBVMainPass::GBVMainPass() {
  shader = Loader.GetShader("::gbvmain");
  // set these textures to null for inspector display
  base = *Loader.GetTexture("::null_texture");
  ILM = *Loader.GetTexture("::null_texture");
  SSS = *Loader.GetTexture("::null_texture");
  detail = *Loader.GetTexture("::null_texture");
}

std::string GBVMainPass::GetMaterialTypeName() { return "GBV Main"; }

void GBVMainPass::drawCustomInspectorGUI() {
  ImGui::BeginChild("gbvmainpasschild", {-1, -1});

  ImGui::MenuItem("Basic Textures", nullptr, nullptr, false);
  GUIUtils::DragableTextureTarget(base, "Base");
  GUIUtils::DragableTextureTarget(ILM, "ILM");
  ImGui::Separator();

  ImGui::MenuItem("Ramp", nullptr, nullptr, false);
  ImGui::SliderFloat("First Ramp Start", &firstRampStart, 0.0f, 1.0f);
  ImGui::SliderFloat("First Ramp Stop", &firstRampStop, 0.0f, 1.0f);
  ImGui::SliderFloat("Ramp Offset", &rampOffset, 0.0f, 1.0f);
  ImGui::SliderFloat("Ramp Shadow Wegith", &rampShadowWeight, 0.0f, 1.0f);
  GUIUtils::DragableTextureTarget(SSS, "SSS");

  ImGui::MenuItem("Specular", nullptr, nullptr, false);
  ImGui::SliderInt("Specular Gloss", &specularGloss, 1, 128);
  ImGui::SliderFloat("Specular Weight", &specularWeight, 0.0f, 2.0f);

  ImGui::MenuItem("Rim Light", nullptr, nullptr, false);
  GUIUtils::ColorEdit3(rimLightColor, "Rim Light Color");
  ImGui::SliderFloat("Rim Light Width", &rimLightWidth, 0.0f, 1.0f);
  ImGui::SliderFloat("Rim Light Smooth", &rimLightSmooth, 0.0f, 0.1f);
  ImGui::Separator();

  ImGui::MenuItem("Details", nullptr, nullptr, false);
  GUIUtils::DragableTextureTarget(detail, "Detail");
  ImGui::SliderFloat("Detail Weight", &detailWeight, 0.0f, 1.0f);
  ImGui::SliderFloat("Inner Line Weight", &innerLineWeight, 0.0f, 1.0f);

  ImGui::EndChild();
}

void GBVMainPass::additionalSetup() {
  shader->Use();
  ActivateTexture2D(base, shader, "Base", 0);
  ActivateTexture2D(ILM, shader, "ILM", 1);
  ActivateTexture2D(SSS, shader, "SSS", 2);
  ActivateTexture2D(detail, shader, "Detail", 3);

  shader->SetFloat("firstRampStart", firstRampStart);
  shader->SetFloat("firstRampStop", firstRampStop);
  shader->SetFloat("rampOffset", rampOffset);
  shader->SetFloat("rampShadowWeight", rampShadowWeight);

  shader->SetVec3("rimLightColor", rimLightColor);
  shader->SetFloat("rimLightWidth", rimLightWidth);
  shader->SetFloat("rimLightSmooth", rimLightSmooth);

  shader->SetInt("specularGloss", specularGloss);
  shader->SetFloat("specularWeight", specularWeight);

  shader->SetFloat("detailWeight", detailWeight);
  shader->SetFloat("innerLineWeight", innerLineWeight);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
}

void GBVMainPass::FinishPass() { glDisable(GL_CULL_FACE); }

}; // namespace Render

}; // namespace aEngine