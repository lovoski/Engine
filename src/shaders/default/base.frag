#version 460 core

uniform vec3 Albedo;
uniform vec3 dLightDir0;
uniform vec3 dLightColor0;

uniform float Ambient;

in vec2 texCoord;
in vec3 normal;

out vec4 FragColor;

void main() {
  vec3 ambient = Ambient * dLightColor0;

  vec3 Normal = normalize(normal);
  vec3 LightDir = -dLightDir0;
  float diff = max(dot(Normal, LightDir), 0.0);
  vec3 diffuse = diff * dLightColor0;

  vec3 result = (ambient + diffuse) * Albedo;

  FragColor = vec4(result, 1.0);
}