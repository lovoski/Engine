#include "Function/Render/RenderPass.hpp"
#include "Entity.hpp"
#include "Function/AssetsLoader.hpp"
#include "Function/GUI/Helpers.hpp"
#include "Scene.hpp"

#include "Component/Light.hpp"

namespace aEngine {

namespace Render {

using glm::vec2;
using glm::vec3;
using std::string;
using std::vector;

void BasePass::SetupLights(Buffer &lightsBuffer,
                           std::shared_ptr<EnvironmentLight> skyLight,
                           int bindingPoint) {
  if (shader != nullptr) {
    shader->Use();
    lightsBuffer.BindToPointAs(GL_SHADER_STORAGE_BUFFER, bindingPoint);
    // setup skylight
    static EnvironmentLight pureBlack(-1);
    if (skyLight != nullptr) {
      shader->SetCubeMap("EnvironmentMap", skyLight->CubeMap, 0);
      shader->SetCubeMap("DiffuseIrradiance", skyLight->Irradiance, 1);
    } else {
      shader->SetCubeMap("EnvironmentMap", pureBlack.CubeMap, 0);
      shader->SetCubeMap("DiffuseIrradiance", pureBlack.CubeMap, 1);
    }
  } else {
    LOG_F(ERROR, "shader not setup for render pass %s, set to error shader",
          identifier.c_str());
    shader = Loader.GetShader("::error");
  }
}

std::string BasePass::getInspectorWindowName() { return typeid(*this).name(); }

void BasePass::DrawInspectorGUIInternal() {
  ImGui::Checkbox("Enable Pass", &Enabled);
  std::strcpy(identifierBuffer.data(), identifier.data());
  if (ImGui::InputText("Identifier##renderpassidentifier",
                       identifierBuffer.data(), identifierBuffer.size())) {
    identifier = identifierBuffer.data();
  }
  ImGui::Separator();
  if (!Enabled)
    ImGui::BeginDisabled();
  DrawInspectorGUI();
  if (!Enabled)
    ImGui::EndDisabled();
}

void BasePass::BeforePassInternal(glm::mat4 &model, glm::mat4 &view,
                                  glm::mat4 &projection, glm::vec3 &viewDir,
                                  bool receiveShadow) {
  if (shader != nullptr) {
    shader->Use();
    glm::mat4 ModelToWorldPoint = model;
    glm::mat3 ModelToWorldDir = glm::mat3(ModelToWorldPoint);
    shader->SetMat4("Projection", projection);
    shader->SetMat4("View", view);
    shader->SetMat4("ModelToWorldPoint", ModelToWorldPoint);
    shader->SetMat3("ModelToWorldDir", ModelToWorldDir);
    shader->SetVec3("ViewDir", viewDir);
    shader->SetVec2("ViewportSize", GWORLD.Context.sceneWindowSize);
    shader->SetInt("ReceiveShadow", receiveShadow);
    BeforePass();
  } else {
    LOG_F(ERROR, "shader not setup for render pass %s, set to error shader",
          identifier.c_str());
    shader = Loader.GetShader("::error");
  }
};

}; // namespace Render

}; // namespace aEngine