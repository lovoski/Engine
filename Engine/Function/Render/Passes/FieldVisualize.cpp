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

out float value;
out vec3 worldPos;
out vec3 worldNormal;

uniform mat4 ModelToWorldPoint;
uniform mat3 ModelToWorldDir;
uniform mat4 View;
uniform mat4 Projection;

void main() {
  value = aTexCoord.x;
  worldPos = (ModelToWorldPoint * aPos).xyz;
  worldNormal = normalize(ModelToWorldDir * aNormal.xyz);
  gl_Position = Projection * View * vec4(worldPos, 1.0);
}
)";
const std::string fvpFS = R"(
#version 430 core

in float value;
in vec3 worldPos;
in vec3 worldNormal;

uniform sampler2D ColorBar;

out vec4 FragColor;

void main() {
  vec3 color = texture(ColorBar, vec2(value, 0.0)).rgb;
  FragColor = vec4(color, 1.0);
}
)";
DiscreteFieldVisualizePass::DiscreteFieldVisualizePass() {
  initTexture();
  shader = Loader.GetShader("::fvp");
}

void DiscreteFieldVisualizePass::BeforePass() {
  shader->Use();
  shader->SetTexture2D(ColorBar, "ColorBar", 0);
}

void DiscreteFieldVisualizePass::FinishPass() {}

void DiscreteFieldVisualizePass::DrawInspectorGUI() {
  GUIUtils::ColorEdit3("Color Begin", BeginColor);
  GUIUtils::ColorEdit3("Color End", EndColor);
  ImGui::Checkbox("With Bar", &WithBar);
  ImGui::DragFloat("Bar Width", &BarWidth, 0.001f, 0.0f, 1.0f);
  ImGui::DragFloat("Bar Interval", &BarInterval, 0.001f, 1e-5f, 1.0f);
  GUIUtils::ColorEdit3("Bar Color", BarColor);
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