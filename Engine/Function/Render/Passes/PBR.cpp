#include "Function/AssetsLoader.hpp"
#include "Function/GUI/Helpers.hpp"
#include "Function/Render/Passes/Header.hpp"
#include "Function/Render/RenderPass.hpp"

namespace aEngine {

namespace Render {

const std::string pbrVS = R"(
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

const std::string pbrFS = R"(
#version 460 core
#extension GL_ARB_bindless_texture : require

struct LightData {
  int meta[4];
  float fmeta[4];
  vec4 color;
  vec4 position;
  vec4 direction;
  mat4 lightMatrix;
  uvec4 shadowMap;
};
layout(std430, binding = 0) buffer Lights {
  LightData lights[];
};

float PI = 3.1415926535;
in vec2 texCoord;
in vec3 worldPos;
in vec3 worldView;
in vec3 worldNormal;

uniform float roughnessFactor;
uniform float metallicFactor;
uniform float aoFactor;
uniform vec3 albedoFactor;
uniform vec3 F0;
uniform sampler2D Albedo;
uniform sampler2D Roughness;
uniform sampler2D Metallic;
uniform sampler2D AO;
uniform bool withNormalMap;
uniform sampler2D Normal;

uniform samplerCube EnvironmentMap;
uniform samplerCube DiffuseIrradiance;

out vec4 FragColor;

vec3 getNormalFromMap()
{
  vec3 tangentNormal = texture(Normal, texCoord).xyz * 2.0 - 1.0;
  vec3 Q1  = dFdx(worldPos);
  vec3 Q2  = dFdy(worldPos);
  vec2 st1 = dFdx(texCoord);
  vec2 st2 = dFdy(texCoord);
  vec3 N   = normalize(worldNormal);
  vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
  vec3 B  = -normalize(cross(N, T));
  mat3 TBN = mat3(T, B, N);
  return normalize(TBN * tangentNormal);
}
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
  float a      = roughness*roughness;
  float a2     = a*a;
  float NdotH  = max(dot(N, H), 0.0);
  float NdotH2 = NdotH*NdotH;

  float num   = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;

  return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
  float r = (roughness + 1.0);
  float k = (r*r) / 8.0;

  float num   = NdotV;
  float denom = NdotV * (1.0 - k) + k;

  return num / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);
  float ggx2  = GeometrySchlickGGX(NdotV, roughness);
  float ggx1  = GeometrySchlickGGX(NdotL, roughness);

  return ggx1 * ggx2;
}
vec3 fresnelSchlick(float cosTheta, vec3 F0, float roughness)
{
  return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
  vec3 N;
  if (withNormalMap)
    N = getNormalFromMap();
  else
    N = normalize(worldNormal);
  vec3 V = normalize(worldView);

  float roughness = roughnessFactor * texture(Roughness, texCoord).r;
  float metallic = metallicFactor * texture(Metallic, texCoord).r;
  float ao = aoFactor * texture(AO, texCoord).r;
  vec3 albedo = albedoFactor * pow(texture(Albedo, texCoord).rgb, vec3(2.2));

  vec3 _F0 = mix(F0, albedo, metallic);

  vec3 Lo = vec3(0.0);
  for (int i = 0; i < lights.length(); ++i) {
    vec3 lightColor = lights[i].color.xyz;
    vec3 L;
    if (lights[i].meta[0] == 0) {
      L = -normalize(lights[i].direction.xyz);
    } else if (lights[i].meta[0] == 1) {
      L = normalize(lights[i].position.xyz-worldPos);
      float distance = length(lights[i].position.xyz - worldPos);
      float attenuation = (lights[i].fmeta[0] * lights[i].fmeta[0]) / (distance * distance);
      lightColor *= attenuation;
    }
    vec3 H = normalize(V + L);
    vec3 radiance = lightColor;

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), _F0, roughness);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
  }

  vec3 ambient = texture(DiffuseIrradiance, N).rgb * albedo * ao;
  vec3 color = ambient + Lo;

  color = color / (color + vec3(1.0));
  color = pow(color, vec3(1.0/2.2));

  FragColor = vec4(color, 1.0);
}
)";

PBRPass::PBRPass() {
  shader = Loader.GetShader("::pbr");
  AO = *Loader.GetTexture("::null_texture");
  Albedo = *Loader.GetTexture("::null_texture");
  Metallic = *Loader.GetTexture("::null_texture");
  Roughness = *Loader.GetTexture("::null_texture");
  Normal = *Loader.GetTexture("::null_texture");
}

void PBRPass::initTexture(std::string p1, std::string p2, std::string p3,
                          std::string p4, std::string p5) {
  Roughness = *Loader.GetTexture(p1);
  Metallic = *Loader.GetTexture(p2);
  AO = *Loader.GetTexture(p3);
  Albedo = *Loader.GetTexture(p4);
  Normal = *Loader.GetTexture(p5);
}

std::string PBRPass::getInspectorWindowName() { return "PBR Pass"; }

void PBRPass::FinishPass() {}

void PBRPass::BeforePass() {
  shader->Use();
  shader->SetFloat("roughnessFactor", RoughnessFactor);
  shader->SetFloat("metallicFactor", MetallicFactor);
  shader->SetFloat("aoFactor", AOFactor);
  shader->SetVec3("albedoFactor", AlbedoFactor);
  shader->SetTexture2D(Albedo, "Albedo", 0);
  shader->SetTexture2D(Metallic, "Metallic", 1);
  shader->SetTexture2D(Roughness, "Roughness", 2);
  shader->SetTexture2D(AO, "AO", 3);
  shader->SetVec3("F0", F0);
  shader->SetBool("withNormalMap", WithNormalMap);
}

void PBRPass::DrawInspectorGUI() {
  GUIUtils::ColorEdit3("Albedo", AlbedoFactor);
  GUIUtils::DragableTextureTarget("Albedo", Albedo);
  ImGui::SliderFloat("Roughness", &RoughnessFactor, 0.0f, 1.0f);
  GUIUtils::DragableTextureTarget("Roughness", Roughness);
  ImGui::SliderFloat("Metallic", &MetallicFactor, 0.0f, 1.0f);
  GUIUtils::DragableTextureTarget("Metallic", Metallic);
  ImGui::SliderFloat("AO", &AOFactor, 0.0f, 1.0f);
  GUIUtils::DragableTextureTarget("AO", AO);
  ImGui::InputFloat3("F0", &F0.x);
  ImGui::Checkbox("With Normal Map", &WithNormalMap);
  if (!WithNormalMap)
    ImGui::BeginDisabled();
  GUIUtils::DragableTextureTarget("Normal", Normal);
  if (!WithNormalMap)
    ImGui::EndDisabled();
}

}; // namespace Render

}; // namespace aEngine

REGISTER_RENDER_PASS_SL(aEngine::Render, PBRPass)