#version 430 core

in vec3 worldNormal;
in vec3 worldPos;
in vec2 texCoord;
in vec3 vertColor;

uniform sampler2D D_Map;
uniform sampler2D M_Map;
uniform sampler2D N_Map;

out vec4 FragColor;

void main() {
  vec4 color = texture(D_Map, texCoord);
  FragColor = vec4(vec3(color.rgb), 1.0);
}