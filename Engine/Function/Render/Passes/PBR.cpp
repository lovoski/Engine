#include "Function/AssetsLoader.hpp"
#include "Function/GUI/Helpers.hpp"
#include "Function/Render/RenderPass.hpp"

namespace aEngine {

namespace Render {

const std::string pbrVS = R"(
#version 460 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec4 aTexCoord;

uniform mat4 ModelToWorldPoint;
uniform mat3 ModelToWorldDir;
uniform mat4 View;
uniform mat4 Projection;
uniform vec3 ViewDir;

out vec2 texCoord;
out vec3 worldPos;
out vec3 worldView;
out vec3 worldNormal;

void main() {
  texCoord = aTexCoord.xy;
  worldView = normalize(ViewDir);
  worldPos = (ModelToWorldPoint * aPos).xyz;
  worldNormal = normalize(ModelToWorldDir * aNormal.xyz);
  gl_Position = Projection * View * vec4(worldPos, 1.0);
}
)";

const std::string pbrFS = R"(
#version 460 core

in vec2 texCoord;
in vec3 worldPos;
in vec3 worldView;
in vec3 worldNormal;

out vec4 FragColor;

void main() {
  vec3 normal = normalize(worldNormal);
  vec3 view = normalize(worldView);
  vec3 halfVector = normalize(normal + view);

  FragColor = vec4(halfVector, 1.0);
}
)";

PBRPass::PBRPass() {
  shader = Loader.GetShader("::pbr");
}

std::string PBRPass::getInspectorWindowName() { return "PBR Pass"; }

void PBRPass::FinishPass() {}

void PBRPass::BeforePass() {}

void PBRPass::DrawInspectorGUI() {
  // static std::vector<std::string> types{"Cook Torrance"};
  // GUIUtils::Combo("Type", types, currentType);
}

}; // namespace Render

}; // namespace aEngine