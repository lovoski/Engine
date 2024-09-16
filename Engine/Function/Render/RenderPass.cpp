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

void BasePass::SetupLights(Buffer &lightsBuffer, int bindingPoint) {
  if (shader == nullptr)
    LOG_F(ERROR, "shader not setup for render pass %s", identifier.c_str());
  shader->Use();
  lightsBuffer.BindToPointAs(GL_SHADER_STORAGE_BUFFER, bindingPoint);
}

std::string BasePass::getInspectorWindowName() { return typeid(*this).name(); }

void BasePass::DrawInspectorGUIInternal() {
  ImGui::Checkbox("Enable Pass", &Enabled);
  ImGui::Separator();
  if (!Enabled)
    ImGui::BeginDisabled();
  DrawInspectorGUI();
  if (!Enabled)
    ImGui::EndDisabled();
}

void BasePass::BeforePassInternal(glm::mat4 &model, glm::mat4 &view,
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
    BeforePass();
  }
};

// ----------------------Basic Material------------------------------

Basic::Basic() {
  // initialize shader to defualt value
  shader = Loader.GetShader("::basic");
}

void Basic::DrawInspectorGUI() {
  ImGui::Checkbox("Wireframe", &withWireframe);
  ImGui::SliderFloat("Width", &WireframeWidth, 0.5f, 5.0f);
  ImGui::SliderFloat("Smooth", &WireframeSmooth, 0.0f, 1.0f);
  GUIUtils::ColorEdit3("Color", WireframeColor);
  ImGui::Separator();
  ImGui::Checkbox("View Normal", &viewNormal);
  ImGui::Separator();
  GUIUtils::ColorEdit3("Albedo", Albedo);
}

void Basic::BeforePass() {
  shader->Use();
  shader->SetVec3("Albedo", Albedo);
  shader->SetFloat("Ambient", Ambient);
  shader->SetVec3("WireframeColor", WireframeColor);
  shader->SetFloat("WireframeWidth", WireframeWidth);
  shader->SetFloat("WireframeSmooth", WireframeSmooth);
  shader->SetBool("Wireframe", withWireframe);
  shader->SetBool("ViewNormal", viewNormal);
}

std::string Basic::getInspectorWindowName() { return "Basic"; }

// ----------------------- Outline -----------------------

OutlinePass::OutlinePass() {
  shader = Loader.GetShader("::outline");
  OutlineColorMap = *Loader.GetTexture("::null_texture");
}

void OutlinePass::DrawInspectorGUI() {
  ImGui::DragFloat("Width", &OutlineWidth, 0.001f, 0.0f, 10.0f);
  ImGui::SliderFloat("Weight", &OutlineWeight, 0.0f, 1.0f);
  float outlineColor[3] = {OutlineColor.x, OutlineColor.y, OutlineColor.z};
  if (ImGui::ColorEdit3("Color", outlineColor)) {
    OutlineColor = glm::vec3(outlineColor[0], outlineColor[1], outlineColor[2]);
  }
  GUIUtils::DragableTextureTarget("Outline Map", OutlineColorMap);
}

void OutlinePass::BeforePass() {
  shader->Use();
  shader->SetFloat("OutlineWidth", OutlineWidth);
  shader->SetFloat("OutlineWeight", OutlineWeight);
  shader->SetVec3("OutlineColor", OutlineColor);
  shader->SetTexture2D(OutlineColorMap, "OutlineMap", 0);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_FRONT);
}

void OutlinePass::FinishPass() { glDisable(GL_CULL_FACE); }

std::string OutlinePass::getInspectorWindowName() { return "Outline"; }

// ----------- Wireframe Shader -----------

WireFramePass::WireFramePass() { shader = Loader.GetShader("::wireframe"); }

void WireFramePass::FinishPass() { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }

std::string WireFramePass::getInspectorWindowName() { return "Wireframe"; }

void WireFramePass::BeforePass() {
  shader->Use();
  shader->SetVec3("wireframeColor", wireFrameColor);
  shader->SetFloat("wireframeOffset", wireframeOffset);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDisable(GL_CULL_FACE);
}

void WireFramePass::DrawInspectorGUI() {
  ImGui::DragFloat("Offset", &wireframeOffset, 0.0001, 0.0f, 10.0f);
  GUIUtils::ColorEdit3("Color", wireFrameColor);
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

std::string GBVMainPass::getInspectorWindowName() { return "GBV Main"; }

void GBVMainPass::DrawInspectorGUI() {
  ImGui::BeginChild("gbvmainpasschild", {-1, -1});

  ImGui::MenuItem("Basic Textures", nullptr, nullptr, false);
  GUIUtils::DragableTextureTarget("Base", base);
  GUIUtils::DragableTextureTarget("ILM", ILM);
  ImGui::Separator();

  ImGui::MenuItem("Ramp", nullptr, nullptr, false);
  ImGui::SliderFloat("First Ramp Start", &firstRampStart, 0.0f, 1.0f);
  ImGui::SliderFloat("First Ramp Stop", &firstRampStop, 0.0f, 1.0f);
  ImGui::SliderFloat("Ramp Offset", &rampOffset, 0.0f, 1.0f);
  ImGui::SliderFloat("Ramp Shadow Wegith", &rampShadowWeight, 0.0f, 1.0f);
  GUIUtils::DragableTextureTarget("SSS", SSS);

  ImGui::MenuItem("Specular", nullptr, nullptr, false);
  ImGui::SliderInt("Specular Gloss", &specularGloss, 1, 128);
  ImGui::SliderFloat("Specular Weight", &specularWeight, 0.0f, 2.0f);

  ImGui::MenuItem("Rim Light", nullptr, nullptr, false);
  GUIUtils::ColorEdit3("Rim Light Color", rimLightColor);
  ImGui::SliderFloat("Rim Light Width", &rimLightWidth, 0.0f, 1.0f);
  ImGui::SliderFloat("Rim Light Smooth", &rimLightSmooth, 0.0f, 0.1f);
  ImGui::Separator();

  ImGui::MenuItem("Details", nullptr, nullptr, false);
  GUIUtils::DragableTextureTarget("Detail", detail);
  ImGui::SliderFloat("Detail Weight", &detailWeight, 0.0f, 1.0f);
  ImGui::SliderFloat("Inner Line Weight", &innerLineWeight, 0.0f, 1.0f);

  ImGui::EndChild();
}

void GBVMainPass::BeforePass() {
  shader->Use();
  shader->SetTexture2D(base, "Base", 0);
  shader->SetTexture2D(ILM, "ILM", 1);
  shader->SetTexture2D(SSS, "SSS", 2);
  shader->SetTexture2D(detail, "Detail", 3);

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

// ----------- PBR shader -------------

const std::string cookTorranceVS = R"(
#version 460 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec4 aTexCoord;

uniform mat4 ModelToWorldPoint;
uniform mat3 ModelToWorldDir;
uniform mat4 View;
uniform mat4 Projection;
uniform vec3 ViewDir;

out vec2 texCoord;
out vec3 worldPos;
out vec3 worldView;
out vec3 worldNormal;

void main() {
  texCoord = aTexCoord.xy;
  worldView = normalize(ViewDir);
  worldPos = (ModelToWorldPoint * aPos).xyz;
  worldNormal = normalize(ModelToWorldDir * aNormal.xyz);
  gl_Position = Projection * View * vec4(worldPos, 1.0);
}
)";

const std::string cookTorranceFS = R"(
#version 460 core

in vec2 texCoord;
in vec3 worldPos;
in vec3 worldView;
in vec3 worldNormal;

out vec4 FragColor;

void main() {
  FragColor = vec4(1.0);
}
)";

PBRPass::PBRPass() {
  cookTorrance = std::make_shared<Shader>();
  cookTorrance->LoadAndRecompileShaderSource(cookTorranceVS, cookTorranceFS);
  shader = cookTorrance.get();
}

std::string PBRPass::getInspectorWindowName() { return "PBR Pass"; }

void PBRPass::FinishPass() {}

void PBRPass::BeforePass() {}

void PBRPass::DrawInspectorGUI() {
  static std::vector<std::string> types{"Cook Torrance"};
  GUIUtils::Combo("Type", types, currentType);
}

}; // namespace Render

}; // namespace aEngine