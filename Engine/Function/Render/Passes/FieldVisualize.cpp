#include "Function/AssetsLoader.hpp"
#include "Function/GUI/Helpers.hpp"
#include "Function/Render/Passes/Header.hpp"
#include "Function/Render/Tools.hpp"

namespace aEngine {

namespace Render {

const std::string fvpVS = R"(
#version 430 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec4 aTexCoord;

out float vfieldValue;
out vec2 vtexCoord;
out vec3 vworldPos;
out vec3 vworldNormal;

uniform mat4 ModelToWorldPoint;
uniform mat3 ModelToWorldDir;
uniform mat4 View;
uniform mat4 Projection;

void main() {
  vfieldValue = aTexCoord.x;
  vtexCoord = aTexCoord.xy;
  vworldPos = (ModelToWorldPoint * aPos).xyz;
  vworldNormal = normalize(ModelToWorldDir * aNormal.xyz);
  gl_Position = Projection * View * vec4(vworldPos, 1.0);
}
)";
const std::string fvpGS = R"(
#version 430 core
layout (triangles) in;
layout (triangle_strip) out;
layout (max_vertices = 3) out;

uniform vec2 ViewportSize;

in float vfieldValue[];
in vec2 vtexCoord[];
in vec3 vworldPos[];
in vec3 vworldNormal[];

out float fieldValue;
out vec3 worldPos;
out vec3 worldNormal;
out vec2 texCoord;
// use linear interpolation instead of perspective interpolation
noperspective out vec3 edgeDistance;

void main() {
  vec3 ndc0 = gl_in[0].gl_Position.xyz / gl_in[0].gl_Position.w;
  vec2 p0;
  p0.x = (ndc0.x + 1.0) / 2.0 * ViewportSize.x;
  p0.y = (ndc0.y + 1.0) / 2.0 * ViewportSize.y;

  vec3 ndc1 = gl_in[1].gl_Position.xyz / gl_in[1].gl_Position.w;
  vec2 p1;
  p1.x = (ndc1.x + 1.0) / 2.0 * ViewportSize.x;
  p1.y = (ndc1.y + 1.0) / 2.0 * ViewportSize.y;

  vec3 ndc2 = gl_in[2].gl_Position.xyz / gl_in[2].gl_Position.w;
  vec2 p2;
  p2.x = (ndc2.x + 1.0) / 2.0 * ViewportSize.x;
  p2.y = (ndc2.y + 1.0) / 2.0 * ViewportSize.y;

  // p0, p1 and p2 are screen positions of the three vertices
  float e01 = length(p0-p1), e12 = length(p1-p2), e20 = length(p2-p0);
  float a1 = acos((e01*e01+e12*e12-e20*e20)/(2*e01*e12));
  float a2 = acos((e12*e12+e20*e20-e01*e01)/(2*e12*e20));
  float h20 = e12*sin(a2), h01 = e12*sin(a1), h12 = e01*sin(a1);

  gl_Position = gl_in[0].gl_Position;
  texCoord = vtexCoord[0];
  worldPos = vworldPos[0];
  worldNormal = vworldNormal[0];
  fieldValue = vfieldValue[0];
  edgeDistance = vec3(h12, 0.0, 0.0);
  EmitVertex();

  gl_Position = gl_in[1].gl_Position;
  texCoord = vtexCoord[1];
  worldPos = vworldPos[1];
  worldNormal = vworldNormal[1];
  fieldValue = vfieldValue[1];
  edgeDistance = vec3(0.0, h20, 0.0);
  EmitVertex();

  gl_Position = gl_in[2].gl_Position;
  texCoord = vtexCoord[2];
  worldPos = vworldPos[2];
  worldNormal = vworldNormal[2];
  fieldValue = vfieldValue[2];
  edgeDistance = vec3(0.0, 0.0, h01);
  EmitVertex();

  EndPrimitive();
}
)";
const std::string fvpFS = R"(
#version 430 core

in float fieldValue;
in vec2 texCoord;
in vec3 worldPos;
in vec3 worldNormal;
in vec3 edgeDistance;

uniform bool Wireframe;
uniform float WireframeWidth;
uniform float WireframeSmooth;
uniform vec3 WireframeColor;

uniform sampler2D ColorBar;

out vec4 FragColor;

void main() {
  vec3 colorBar = texture(ColorBar, vec2(fieldValue, 0.0)).rgb;
  if (Wireframe) {
    float d = min(edgeDistance.x, min(edgeDistance.y, edgeDistance.z));
    float alpha = 0.0;
    if (d < WireframeWidth - WireframeSmooth) {
      alpha = 1.0;
    } else if (d > WireframeWidth + WireframeSmooth) {
      alpha = 0.0;
    } else {
      float x = d - (WireframeWidth - WireframeSmooth);
      alpha = exp2(-2.0 * x * x);
    }
    colorBar = mix(colorBar, WireframeColor, alpha);
  }
  FragColor = vec4(colorBar, 1.0);
}
)";
DiscreteFieldVisualizePass::DiscreteFieldVisualizePass() {
  initTexture();
  shader = Loader.GetShader("::fvp");
}

void DiscreteFieldVisualizePass::BeforePass() {
  shader->Use();
  shader->SetTexture2D(ColorBar, "ColorBar", 0);
  shader->SetBool("Wireframe", Wireframe);
  shader->SetFloat("WireframeWidth", WireframeWidth);
  shader->SetFloat("WireframeSmooth", WireframeSmooth);
  shader->SetVec3("WireframeColor", WireframeColor);
}

void DiscreteFieldVisualizePass::FinishPass() {}

void DiscreteFieldVisualizePass::DrawInspectorGUI() {
  GUIUtils::ColorEdit3("Color Begin", BeginColor);
  GUIUtils::ColorEdit3("Color End", EndColor);
  ImGui::Checkbox("With Bar", &WithBar);
  ImGui::DragFloat("Bar Width", &BarWidth, 0.001f, 0.0f, 1.0f);
  ImGui::DragFloat("Bar Interval", &BarInterval, 0.001f, 1e-5f, 1.0f);
  GUIUtils::ColorEdit3("Bar Color", BarColor);
  ImGui::Separator();
  ImGui::Checkbox("Wireframe", &Wireframe);
  ImGui::SliderFloat("Wireframe Width", &WireframeWidth, 0.5f, 5.0f);
  ImGui::SliderFloat("Wireframe Smooth", &WireframeSmooth, 0.0f, 1.0f);
  GUIUtils::ColorEdit3("Wireframe Color", WireframeColor);
  auto maxWidth = ImGui::GetContentRegionAvail().x;
  if (ImGui::Button("Build Color Bar", {-1, 30}))
    BuildColorBar();
  ImGui::Image((void *)static_cast<std::uintptr_t>(ColorBar), {maxWidth, 32},
               ImVec2(0, 1), ImVec2(1, 0));
}

void DiscreteFieldVisualizePass::initTexture() {
  WhiteTexture = Loader.GetTexture("::white_texture")->id;
  ColorBar = WhiteTexture;
}

const std::string buildColorBarFS = R"(
#version 430 core

in vec2 texCoord;
uniform vec3 BeginColor;
uniform vec3 EndColor;
uniform bool WithBar;
uniform float BarWidth;
uniform float BarInterval;
uniform vec3 BarColor;

out vec4 FragColor;

void main() {
  float x = texCoord.x;
  vec3 color = mix(BeginColor, EndColor, x);
  if (WithBar) {
    float center = 0.0, interval = BarInterval;
    if (interval < 1e-5)
      interval = 1e-5;
    while (center < 1.0) {
      if (abs(x-center)<0.5*BarWidth) {
        color = mix(BarColor, color, pow(2*abs(x-center)/BarWidth, 3));
        break;
      }
      center += interval;
    }
  }
  FragColor = vec4(color, 1.0);
}
)";
void DiscreteFieldVisualizePass::BuildColorBar() {
  static Shader shader;
  static bool initialized = false;
  if (!initialized) {
    shader.LoadAndRecompileShaderSource(defaultProgramTextureVS,
                                        buildColorBarFS);
    initialized = true;
  }
  shader.Use();
  shader.SetVec3("BeginColor", BeginColor);
  shader.SetVec3("EndColor", EndColor);
  shader.SetBool("WithBar", WithBar);
  shader.SetFloat("BarInterval", BarInterval);
  shader.SetFloat("BarWidth", BarWidth);
  shader.SetVec3("BarColor", BarColor);
  ProgramTexture(shader, ColorBar, ColorBarWidth, ColorBarHeight);
}

}; // namespace Render

}; // namespace aEngine

REGISTER_RENDER_PASS_SL(aEngine::Render, DiscreteFieldVisualizePass)