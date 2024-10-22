#include "Function/AssetsLoader.hpp"
#include "Function/GUI/Helpers.hpp"
#include "Function/Render/RenderPass.hpp"
#include "Function/Render/Passes/Header.hpp"

namespace aEngine {

namespace Render {

const std::string GBVMainVS = R"(
#version 430 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec4 aTexCoord;
layout (location = 3) in vec4 aColor;

out vec2 texCoord1;
out vec2 texCoord2;
out vec3 worldPos;
out vec3 worldNormal;
out vec3 worldViewDir;
out vec3 worldReflect;
out vec4 vertColor;

uniform mat4 ModelToWorldPoint;
uniform mat3 ModelToWorldDir;
uniform mat4 View;
uniform mat4 Projection;
uniform vec3 ViewDir;

void main() {
  vertColor = aColor;
  texCoord1 = aTexCoord.xy;
  texCoord2 = aTexCoord.zw;
  worldNormal = normalize(ModelToWorldDir * vec3(aNormal));
  worldPos = (ModelToWorldPoint * aPos).xyz;
  worldViewDir = normalize(ViewDir);
  worldReflect = reflect(-worldViewDir, worldNormal);
  gl_Position = Projection * View * vec4(worldPos, 1.0);
}
)";
const std::string GBVMainFS = R"(
#version 430 core
struct LightData {
  int meta[4];
  float fmeta[4];
  vec4 color;
  vec4 position;
  vec4 direction;
  mat4 lightMatrix;
};
layout(std430, binding = 0) buffer Lights {
  LightData lights[];
};

uniform int ReceiveShadow;

uniform sampler2D Base;
uniform sampler2D ILM;
uniform sampler2D SSS;
uniform sampler2D Detail;

uniform float firstRampStart;
uniform float firstRampStop;
uniform float rampOffset;
uniform float rampShadowWeight;

uniform vec3 rimLightColor;
uniform float rimLightWidth;
uniform float rimLightSmooth;

uniform int specularGloss;
uniform float specularWeight;

uniform float detailWeight;
uniform float innerLineWeight;

in vec4 vertColor;

in vec2 texCoord1;
in vec2 texCoord2;
in vec3 worldPos;
in vec3 worldNormal;
in vec3 worldReflect;
in vec3 worldViewDir;

out vec4 FragColor;

vec3 LightAttenuate(vec3 color, float distance, float intensity) {
  // attenuation for point light
  float constant = 1.0;
  float linear = 0.09;
  float quadratic = 0.032;
  float atten = intensity / (constant + linear * distance + quadratic * distance * distance);
  return color * atten;
}

void main() {
  vec4 result = vec4(0.0, 0.0, 0.0, 1.0);
  vec3 Normal = normalize(worldNormal);
  for (int i = 0; i < lights.length(); ++i) {
    vec3 LightColor = lights[i].color.xyz;
    vec3 LightDir;
    if (lights[i].meta[0] == 0) {
      LightDir = -normalize(lights[i].direction.xyz);
    } else if (lights[i].meta[0] == 1) {
      LightDir = normalize(lights[i].position.xyz-worldPos);
      float distance = length(lights[i].position.xyz-worldPos);
      LightColor = LightAttenuate(LightColor, distance, lights[i].fmeta[0]);
    }

    vec4 baseCol = texture(Base, texCoord1);
    vec4 sssCol = texture(SSS, texCoord1);
    vec4 ilmChannels = texture(ILM, texCoord1);
    float detail = texture(Detail, texCoord2).r;

    float halfLambert = dot(Normal, LightDir) * 0.5 + 0.5;
    float lightThresholdTerm = smoothstep(firstRampStart, firstRampStop, halfLambert * ilmChannels.g * rampOffset);
    lightThresholdTerm *= vertColor.r;
    vec3 rampShadow = mix(sssCol * rampShadowWeight, baseCol, lightThresholdTerm).rgb;

    vec3 diffuse = LightColor.rgb * rampShadow;
    diffuse = mix(diffuse, diffuse * ilmChannels.a, innerLineWeight);
    diffuse = mix(diffuse, diffuse * detail, detailWeight);

    vec3 halfDir = normalize(worldViewDir + LightDir);
    float specularMaskTerm = ilmChannels.r * ilmChannels.b;
    vec3 specular = specularMaskTerm * max(0, pow(dot(halfDir, Normal), specularGloss)) * baseCol.rgb * LightColor.rgb * specularWeight;

    vec3 rimLightCol = smoothstep(1-rimLightWidth-rimLightSmooth, 1-rimLightWidth+rimLightSmooth, 1-dot(Normal, worldViewDir)) * baseCol.a * rimLightColor * baseCol.rgb;
    rimLightCol *= LightColor.rgb;
    rimLightCol = mix(vec3(0.0), rimLightCol, lightThresholdTerm);
    diffuse += rimLightCol;

    result.rgb += specular + diffuse;
  }

  FragColor = result;
}
)";

GBVMainPass::GBVMainPass() {
  shader = Loader.GetShader("::gbvmain");
  // set these textures to null for inspector display
  base = *Loader.GetTexture("::null_texture");
  ILM = *Loader.GetTexture("::null_texture");
  SSS = *Loader.GetTexture("::null_texture");
  detail = *Loader.GetTexture("::null_texture");
}

std::string GBVMainPass::getInspectorWindowName() { return "GBV Main"; }

void GBVMainPass::initTexture(std::string p1, std::string p2, std::string p3,
  std::string p4) {
  base = *Loader.GetTexture(p1);
  ILM = *Loader.GetTexture(p2);
  SSS = *Loader.GetTexture(p3);
  detail = *Loader.GetTexture(p4);
}

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

}; // namespace Render

}; // namespace aEngine

REGISTER_RENDER_PASS_SL(aEngine::Render, GBVMainPass)