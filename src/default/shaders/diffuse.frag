#version 460 core

uniform vec3 Albedo;
uniform vec3 dLightDir0;
uniform vec3 dLightColor0;

uniform vec3 ViewDir;

uniform int Smoothness;
uniform float Ambient;
uniform vec3 Specular;

in vec2 texCoord;
in vec3 normal;

out vec4 FragColor;

void main() {
  // ambient
  vec3 ambient = Ambient * dLightColor0;

  // diffuse
  vec3 Normal = normalize(normal);
  vec3 LightDir = -dLightDir0;
  float lambert = (dot(Normal, LightDir) + 1.0) * 0.5;
  vec3 diffuse = lambert * dLightColor0 * Albedo;

  // specular
  vec3 reflectDir = reflect(-dLightDir0, Normal);
  float spec = pow(max(dot(ViewDir, reflectDir), 0.0), Smoothness);
  vec3 specular = dLightColor0 * (spec * Specular);

  vec3 result = ambient + diffuse;

  FragColor = vec4(result, 1.0);
}