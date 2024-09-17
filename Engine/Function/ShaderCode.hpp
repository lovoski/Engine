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

const std::string skyboxVS = R"(
#version 330 core
layout (location = 0) in vec4 aPos;

out vec3 texCoord;

uniform mat4 VP;

void main() {
  texCoord = aPos.xyz;
  gl_Position = VP * aPos;
}  
)";
const std::string skyboxFS = R"(
#version 330 core
out vec4 FragColor;

in vec3 texCoord;
uniform samplerCube skybox;

void main() {
  FragColor = texture(skybox, texCoord);
}
)";

const std::string sampleHDRVS = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 localPos;

uniform mat4 VP;

void main() {
  localPos = aPos;
  gl_Position = VP * vec4(aPos, 1.0);
}
)";
const std::string sampleHDRFS = R"(
#version 330 core
out vec4 FragColor;
in vec3 localPos;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(102591, 0.3183);
vec2 sampleSphericalMap(vec3 v) {
  vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
  uv *= invAtan;
  uv += 0.5;
  return uv;
}

void main() {
  vec2 uv = sampleSphericalMap(normalize(localPos));
  vec2 color = texture(equirectangularMap, uv).rgb;
  FragColor = vec4(color, 1.0);
}
)";
