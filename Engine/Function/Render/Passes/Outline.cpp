#include "Function/AssetsLoader.hpp"
#include "Function/GUI/Helpers.hpp"
#include "Function/Render/RenderPass.hpp"

namespace aEngine {

namespace Render {

const std::string outlineVS = R"(
#version 460 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec4 aTexCoord;

uniform mat4 ModelToWorldPoint;
uniform mat3 ModelToWorldDir;
uniform mat4 View;
uniform mat4 Projection;

uniform vec3 ViewDir;

uniform float OutlineWidth;

out vec2 texCoord;

void main() {
  texCoord.xy = aTexCoord.xy;
  vec3 worldNormal = normalize(ModelToWorldDir * aNormal.xyz);
  vec4 worldPos = ModelToWorldPoint * aPos;
  worldPos = worldPos + vec4(worldNormal, 0.0) * OutlineWidth;
  gl_Position = Projection * View * worldPos;
}
)";

const std::string outlineFS = R"(
#version 430 core

uniform vec3 OutlineColor;
uniform sampler2D OutlineMap;
uniform float OutlineWeight;

in vec2 texCoord;
out vec4 FragColor;

void main() {
  vec3 result = mix(OutlineColor, texture(OutlineMap, texCoord).xyz, OutlineWeight);
  FragColor = vec4(result, 1.0);
}
)";

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

}; // namespace Render

}; // namespace aEngine