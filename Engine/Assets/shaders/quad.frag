#version 460 core

out vec4 FragColor;

in vec2 texCoord;
uniform sampler2D bufTex;

void main() {
  FragColor = texture(bufTex, texCoord);
}