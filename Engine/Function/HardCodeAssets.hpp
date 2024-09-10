#pragma once

#include "Global.hpp"

const std::string errorVS = R"(
#version 460 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec4 aTexCoord;
uniform mat4 ModelToWorldPoint;
uniform mat4 View;
uniform mat4 Projection;
void main() {
  gl_Position = Projection * View * ModelToWorldPoint * aPos;
}
)";
const std::string errorFS = R"(
#version 460 core
out vec4 FragColor;
void main() {
  FragColor = vec4(0.702, 0.2314, 0.7725, 1.0);
}
)";

// the default diffuse shader
const std::string diffuseVS = R"(
#version 460 core
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
const std::string diffuseGS = R"(
#version 460
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
const std::string diffuseFS = R"(
#version 460
#extension GL_ARB_bindless_texture : require

struct LightData {
  int meta[4];
  float fmeta[4];
  vec4 color;
  vec4 position; // for point light
  vec4 direction; // for directional light
  mat4 lightMatrix; // light space transform matrix
  uvec4 shadowMap; // xy -> shadow map
};
layout(std430, binding = 0) buffer Lights {
  LightData lights[];
};

uniform int ReceiveShadow;
uniform vec3 Albedo;
uniform float Ambient;

uniform bool Wireframe;
uniform float WireframeWidth;
uniform vec3 WireframeColor;

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

int pcfKernelSize = 2;
float ShadowAtten(int lightIndex, float bias) {
  vec3 projCoords = (lights[lightIndex].lightMatrix * vec4(worldPos, 1.0)).xyz;
  projCoords = projCoords * 0.5 + 0.5;
  sampler2D shadowMap = sampler2D(lights[lightIndex].shadowMap.xy);
  float closestDepth = texture(shadowMap, projCoords.xy).r;
  float currentDepth = projCoords.z;
  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
  for(int x = -pcfKernelSize; x <= pcfKernelSize; ++x) {
    for(int y = -pcfKernelSize; y <= pcfKernelSize; ++y) {
      float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
      shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;
    }
  }
  shadow /= pcfKernelSize * pcfKernelSize * 4;
  if (currentDepth > 1.0)
    shadow = 1.0;
  return 1.0 - shadow;
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
      LightColor = LightAttenuate(LightColor, distance, lights[i].fmeta[0]);
    }
    float lambert = (dot(Normal, LightDir) + 1.0) * 0.5;
    vec3 LightEffect = lambert * LightColor;
    if (lights[i].meta[1] == 1 && ReceiveShadow != 0) {
      float bias = max(0.05 * (1.0 - dot(Normal, LightDir)), 0.005);
      LightEffect *= ShadowAtten(i, bias);
    }
    Diffuse = Diffuse + LightEffect;
  }
  return Diffuse;
}

void main() {
  vec3 diffuse = LitSurface() * Albedo;

  vec3 result;
  // wireframe related
  if (Wireframe) {
    float d = min(edgeDistance.x, min(edgeDistance.y, edgeDistance.z));
    float alpha = 0.0;
    if (d < WireframeWidth - 0.5) {
      alpha = 1.0;
    } else if (d > WireframeWidth + 0.5) {
      alpha = 0.0;
    } else {
      float x = d - (WireframeWidth - 0.5);
      alpha = exp2(-2.0 * x * x);
    }
    result = mix(diffuse, WireframeColor, alpha);
  } else {
    result = diffuse;
  }
  FragColor = vec4(result, 1.0);
}
)";

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

const std::string shadowMapDirLightVS = R"(
#version 430 core
layout (location = 0) in vec4 aPos;

uniform mat4 LightSpaceMatrix;
uniform mat4 Model;

void main() {
  gl_Position = LightSpaceMatrix * Model * aPos;
}
)";
const std::string shadowMapDirLightFS = R"(
#version 430 core
void main() {}
)";

const std::string GBVMainVS = R"(
#version 460 core
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
#version 460
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

const std::string wireframeVS = R"(
#version 460 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec4 aTexCoord;

uniform mat4 ModelToWorldPoint;
uniform mat3 ModelToWorldDir;
uniform mat4 View;
uniform mat4 Projection;

uniform float wireframeOffset;

void main() {
  gl_Position = Projection * View * ModelToWorldPoint * (aPos + normalize(aNormal) * wireframeOffset);
}
)";

const std::string wireframeFS = R"(
#version 460 core

uniform vec3 wireframeColor;

out vec4 FragColor;

void main() {
  FragColor = vec4(wireframeColor, 1.0);
}
)";
