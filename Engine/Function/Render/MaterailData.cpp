#include "Entity.hpp"
#include "Scene.hpp"
#include "Function/AssetsLoader.hpp"
#include "Function/Render/MaterialData.hpp"

namespace aEngine {

namespace Render {

using glm::vec2;
using glm::vec3;
using std::string;
using std::vector;

bool ActivateTexture2D(Texture &texture, Shader *shader, string name,
                       int slot) {
  shader->Use(); // activate the shader
  if (texture.path != "::NULL_ICON") {
    // only activate none icon texture
    glActiveTexture(GL_TEXTURE0 + slot);
    int location = glGetUniformLocation(shader->ID, name.c_str());
    if (location == -1)
      return false;
    glUniform1i(location, texture.id);
    glBindTexture(GL_TEXTURE_2D, texture.id);
  }
  return true;
}

void BaseMaterial::SetupLights(vector<std::shared_ptr<Light>> &lights) {
  if (shader == nullptr)
    Console.Log("[error]: shader not setup for material %s\n",
                identifier.c_str());
  unsigned int dirLightCounter = 0;
  unsigned int pointLightCounter = 0;
  unsigned int spotLightCounter = 0;
  // set the properties of different lights
  for (auto light : lights) {
    if (light->type == LIGHT_TYPE::DIRECTIONAL_LIGHT) {
      string lightDirName = "dLightDir" + std::to_string(dirLightCounter);
      string lightColorName = "dLightColor" + std::to_string(dirLightCounter);
      shader->SetVec3(lightDirName,
                      GWORLD.EntityFromID(light->GetID())->LocalForward);
      shader->SetVec3(lightColorName, light->lightColor);
      dirLightCounter++;
    } else if (light->type == LIGHT_TYPE::POINT_LIGHT) {
      string lightPosName = "pLightPos" + std::to_string(pointLightCounter);
      string lightColorName = "pLightColor" + std::to_string(pointLightCounter);
      shader->SetVec3(lightPosName,
                      GWORLD.EntityFromID(light->GetID())->Position());
      shader->SetVec3(lightColorName, light->lightColor);
      pointLightCounter++;
    } else if (light->type == LIGHT_TYPE::SPOT_LIGHT) {
    }
  }
  if (dirLightCounter == 0) {
    Console.Log("[error]: At least one directional light needed for the "
                "default shader\n");
  }
}

std::string BaseMaterial::getMaterialTypeName() { return typeid(*this).name(); }

void BaseMaterial::drawInspectorGUIDefault() {
  ImGui::MenuItem("Name:", nullptr, nullptr, false);
  ImGui::TextWrapped("%s", identifier.c_str());
  ImGui::MenuItem("Type:", nullptr, nullptr, false);
  ImGui::TextWrapped("%s", getMaterialTypeName().c_str());
  ImGui::MenuItem("Properties:", nullptr, nullptr, false);
}

void BaseMaterial::SetVariables(glm::mat4 &model, glm::mat4 &view,
                                glm::mat4 &projection, glm::vec3 &viewDir) {
  if (shader != nullptr) {
    shader->Use();
    glm::mat4 ModelToWorldPoint = model;
    glm::mat3 ModelToWorldDir = glm::mat3(ModelToWorldPoint);
    shader->SetMat4("Projection", projection);
    shader->SetMat4("View", view);
    shader->SetMat4("ModelToWorldPoint", ModelToWorldPoint);
    shader->SetMat3("ModelToWorldDir", ModelToWorldDir);
    shader->SetVec3("ViewDir", viewDir);
    setupCustomVariables();
  }
};

// ----------------------Diffuse Material------------------------------

DiffuseMaterial::DiffuseMaterial() {
  // initialize shader to defualt value
  shader = Loader.GetShader("::diffuse");
}

void DiffuseMaterial::DrawInspectorGUI() {
  drawInspectorGUIDefault();
  ImGui::SliderFloat("Ambient", &Ambient, 0.0f, 1.0f);
  float albedoColor[3] = {Albedo.x, Albedo.y, Albedo.z};
  if (ImGui::ColorEdit3("Albedo", albedoColor)) {
    Albedo = glm::vec3(albedoColor[0], albedoColor[1], albedoColor[2]);
  }
}

void DiffuseMaterial::setupCustomVariables() {
  shader->Use();
  shader->SetVec3("Albedo", Albedo);
  shader->SetFloat("Ambient", Ambient);
}

void DiffuseMaterial::Serialize(Json &json) {
  json["matType"] = "diffuse";
  json["Albedo"] = Albedo;
  json["Ambient"] = Ambient;
}
void DiffuseMaterial::Deserialize(Json &json) {
  Ambient = json.value("Ambient", 0.1f);
  Albedo = json.value("Albedo", glm::vec3(1.0f));
}

}; // namespace Render

}; // namespace aEngine