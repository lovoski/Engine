#include "Function/AssetsLoader.hpp"
#include "Function/GUI/Helpers.hpp"
#include "Function/Render/RenderPass.hpp"
#include "Function/Render/Passes/Header.hpp"

namespace aEngine {

namespace Render {

// the default diffuse shader
const std::string basicVS = R"(
#version 430 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec4 aTexCoord;

out vec3 vworldPos;
out vec3 vworldNormal;
out vec2 vtexCoord;

uniform mat4 ModelToWorldPoint;
uniform mat3 ModelToWorldDir;
uniform mat4 View;
uniform mat4 Projection;
void main() {
  vtexCoord = aTexCoord.xy;
  vworldNormal = ModelToWorldDir * vec3(aNormal);
  vworldPos = (ModelToWorldPoint * aPos).xyz;
  gl_Position = Projection * View * vec4(vworldPos, 1.0);
}
)";
const std::string basicGS = R"(
#version 430 core
layout (triangles) in;
layout (triangle_strip) out;
layout (max_vertices = 3) out;

uniform vec2 ViewportSize;

in vec2 vtexCoord[];
in vec3 vworldPos[];
in vec3 vworldNormal[];

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
  edgeDistance = vec3(h12, 0.0, 0.0);
  EmitVertex();

  gl_Position = gl_in[1].gl_Position;
  texCoord = vtexCoord[1];
  worldPos = vworldPos[1];
  worldNormal = vworldNormal[1];
  edgeDistance = vec3(0.0, h20, 0.0);
  EmitVertex();

  gl_Position = gl_in[2].gl_Position;
  texCoord = vtexCoord[2];
  worldPos = vworldPos[2];
  worldNormal = vworldNormal[2];
  edgeDistance = vec3(0.0, 0.0, h01);
  EmitVertex();

  EndPrimitive();
}
)";
const std::string basicFS = R"(
#version 430 core

struct LightData {
  int meta[4];
  float fmeta[4];
  vec4 color;
  vec4 position; // for point light
  vec4 direction; // for directional light
  mat4 lightMatrix; // light space transform matrix
};
layout(std430, binding = 0) buffer Lights {
  LightData lights[];
};

uniform int ReceiveShadow;
uniform vec3 Albedo;
uniform float Ambient;

uniform bool Wireframe;
uniform float WireframeWidth;
uniform float WireframeSmooth;
uniform vec3 WireframeColor;

uniform bool ViewNormal;

in vec2 texCoord;
in vec3 worldPos;
in vec3 worldNormal;
in vec3 edgeDistance;
out vec4 FragColor;


vec3 LightAttenuate(vec3 color, float distance, float intensity) {
  // attenuation for point light
  float constant = 1.0;
  float linear = 0.09;
  float quadratic = 0.032;
  float atten = intensity / (constant + linear * distance + quadratic * distance * distance);
  return color * atten;
}

vec3 LitSurface() {
  vec3 Diffuse = vec3(0.0, 0.0, 0.0);
  vec3 Normal = normalize(worldNormal);
  for (int i = 0; i < lights.length(); ++i) {
    vec3 LightColor = lights[i].color.xyz;
    vec3 LightDir;
    if (lights[i].meta[0] == 0) {
      LightDir = -normalize(lights[i].direction.xyz);
    } else if (lights[i].meta[0] == 1) {
      LightDir = normalize(lights[i].position.xyz-worldPos);
      float distance = length(lights[i].position.xyz-worldPos);
      LightColor *= (lights[i].fmeta[0] * lights[i].fmeta[0]) / (distance * distance);
    }
    float lambert = (dot(Normal, LightDir) + 1.0) * 0.5;
    vec3 LightEffect = lambert * LightColor;
    if (lights[i].meta[1] == 1 && ReceiveShadow != 0) {
      // float bias = max(0.05 * (1.0 - dot(Normal, LightDir)), 0.005);
      // LightEffect *= ShadowAtten(i, bias);
    }
    Diffuse = Diffuse + LightEffect;
  }
  return Diffuse;
}

void main() {
  vec3 shade;
  if (ViewNormal) {
    shade = normalize(worldNormal);
  } else {
    shade = LitSurface() * Albedo;
  }

  vec3 result;
  // wireframe related
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
    result = mix(shade, WireframeColor, alpha);
  } else {
    result = shade;
  }
  result += Ambient * Albedo;
  FragColor = vec4(result, 1.0);
}
)";

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
  ImGui::SliderFloat("Ambient", &Ambient, 0.0f, 1.0f);
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
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
}

void Basic::FinishPass() {}

std::string Basic::getInspectorWindowName() { return "Basic"; }

}; // namespace Render

}; // namespace aEngine

REGISTER_RENDER_PASS(aEngine::Render, Basic)