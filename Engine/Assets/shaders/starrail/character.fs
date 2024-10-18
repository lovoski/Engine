#version 430 core

in vec3 worldNormal;
in vec3 worldPos;
in vec2 texCoord;
in vec4 vertColor;

uniform sampler2D D_Map;
uniform sampler2D M_Map;
uniform sampler2D N_Map;

out vec4 FragColor;

void main() {
  FragColor = vec4(vec3(vertColor.b), 1.0);
}