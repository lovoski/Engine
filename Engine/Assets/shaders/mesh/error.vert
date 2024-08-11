#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 ModelToWorldPoint;
uniform mat4 View;
uniform mat4 Projection;

void main() {
  gl_Position = Projection * View * ModelToWorldPoint * vec4(aPos, 1.0);
}