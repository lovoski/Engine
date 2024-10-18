#version 430 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec4 aTexCoord;
layout (location = 3) in vec4 aColor;

uniform mat4 ModelToWorldPoint;
uniform mat3 ModelToWorldDir;
uniform mat4 View;
uniform mat4 Projection;
uniform vec3 ViewDir;

out vec3 worldNormal;
out vec3 worldPos;
out vec2 texCoord;
out vec4 vertColor;

void main() {
  vertColor = aColor;
  texCoord = aTexCoord.xy;
  worldPos = (ModelToWorldPoint * aPos).xyz;
  worldNormal = ModelToWorldDir * (aNormal.xyz);
  gl_Position = Projection * View * vec4(worldPos, 1.0);
}