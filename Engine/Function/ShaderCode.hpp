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