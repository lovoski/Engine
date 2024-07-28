#version 460 core

uniform vec3 Albedo;

in vec2 texCoord;

out vec4 FragColor;

void main() {
  FragColor = vec4(Albedo, 1.0);
}