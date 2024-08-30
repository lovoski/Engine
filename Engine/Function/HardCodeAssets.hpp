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

out vec3 normal;
out vec3 worldPos;

// transform point from model space to world space
uniform mat4 ModelToWorldPoint;
// transform vector from model space to world space
uniform mat3 ModelToWorldDir;
// transform world space to camera space
uniform mat4 View;
// transform camera space to screen
uniform mat4 Projection;
void main() {
  normal = ModelToWorldDir * vec3(aNormal);
  worldPos = (ModelToWorldPoint * aPos).xyz;
  gl_Position = Projection * View * vec4(worldPos, 1.0);
}
)";

const std::string diffuseFS = R"(
#version 430 core

struct LightData {
  // [0]: 0 for directional light, 1 for point light
  // [1]: 0 for not receive shadow, 1 for receive shadow
  int meta[4];
  vec4 color;
  vec4 position; // for point light
  vec4 direction; // for directional light
  mat4 lightMatrix; // light space transform matrix
};
layout(std430, binding = 0) buffer Lights {
  LightData lights[];
};

uniform vec3 Albedo;
uniform float Ambient;

in vec3 normal;
in vec3 worldPos;
out vec4 FragColor;

float ShadowAtten() {
  return 1.0;
}

// attenuation for point light
float constant = 1.0;
float linear = 0.09;
float quadratic = 0.032;

vec3 LightAttenuate(vec3 color, float distance) {
  float atten = 1.0 / (constant + linear * distance + quadratic * distance * distance);
  return color * atten;
}

vec3 LitSurface() {
  vec3 Normal = normalize(normal);
  vec3 Diffuse = vec3(0.0, 0.0, 0.0);
  for (int i = 0; i < lights.length(); ++i) {
    vec3 LightColor = lights[i].color.xyz;
    vec3 LightDir;
    if (lights[i].meta[0] == 0) {
      LightDir = -normalize(lights[i].direction.xyz);
    } else if (lights[i].meta[0] == 1) {
      LightDir = normalize(lights[i].position.xyz-worldPos);
      float distance = length(lights[i].position.xyz-worldPos);
      LightColor = LightAttenuate(LightColor, distance);
    }
    float lambert = (dot(Normal, LightDir) + 1.0) * 0.5;
    Diffuse = Diffuse + lambert * LightColor;
  }
  return Diffuse;
}

void main() {
  // diffuse
  vec3 diffuse = LitSurface() * Albedo;
  // vec3 result = ambient + diffuse;
  vec3 result = diffuse;
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

void main() {
  vec3 worldNormal = normalize(ModelToWorldDir * aNormal.xyz);
  vec4 worldPos = ModelToWorldPoint * aPos;
  worldPos = worldPos + vec4(worldNormal, 0.0) * OutlineWidth;
  gl_Position = Projection * View * worldPos;
}
)";

const std::string outlineFS = R"(
#version 430 core

uniform vec3 OutlineColor;

out vec4 FragColor;

void main() {
  FragColor = vec4(OutlineColor, 1.0);
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
