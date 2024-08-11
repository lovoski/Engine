#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec2 texCoord;
out vec3 normal;

// transform point from model space to world space
uniform mat4 ModelToWorldPoint;
// transform vector from model space to world space
uniform mat3 ModelToWorldDir;
// transform world space to camera space
uniform mat4 View;
// transform camera space to screen
uniform mat4 Projection;

void main() {
  normal = ModelToWorldDir * aNormal;
  texCoord = aTexCoord;
  gl_Position = Projection * View * ModelToWorldPoint * vec4(aPos, 1.0);
}