#include "Function/AssetsLoader.hpp"
#include "Function/GUI/Helpers.hpp"
#include "Function/Render/RenderPass.hpp"
#include "Function/Render/Passes/Header.hpp"

namespace aEngine {

namespace Render {

const std::string wireframeVS = R"(
#version 430 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec4 aTexCoord;

uniform mat4 ModelToWorldPoint;
uniform mat3 ModelToWorldDir;
uniform mat4 View;
uniform mat4 Projection;

uniform float wireframeOffset;

void main() {
  gl_Position = Projection * View * ModelToWorldPoint * (aPos + normalize(aNormal) * wireframeOffset);
}
)";

const std::string wireframeFS = R"(
#version 430 core

uniform vec3 wireframeColor;

out vec4 FragColor;

void main() {
  FragColor = vec4(wireframeColor, 1.0);
}
)";

WireFramePass::WireFramePass() {
  shader = Loader.GetShader("::wireframe");
}

void WireFramePass::FinishPass() { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); }

std::string WireFramePass::getInspectorWindowName() { return "Wireframe"; }

void WireFramePass::BeforePass() {
  shader->Use();
  shader->SetVec3("wireframeColor", wireFrameColor);
  shader->SetFloat("wireframeOffset", wireframeOffset);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDisable(GL_CULL_FACE);
}

void WireFramePass::DrawInspectorGUI() {
  ImGui::DragFloat("Offset", &wireframeOffset, 0.0001, 0.0f, 10.0f);
  GUIUtils::ColorEdit3("Color", wireFrameColor);
}

}; // namespace Render

}; // namespace aEngine

REGISTER_RENDER_PASS(aEngine::Render, WireFramePass)