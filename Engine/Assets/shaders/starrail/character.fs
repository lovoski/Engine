#version 430 core

in vec3 worldNormal;
in vec3 worldPos;
in vec2 texCoord;
in vec4 vertColor;

uniform sampler2D DiffuseMap;
uniform sampler2D LightMap;
uniform sampler2D RampTex;

out vec4 FragColor;

void main() {
  vec3 color = texture(DiffuseMap, texCoord).rgb;
  FragColor = vec4(color, 1.0);
}