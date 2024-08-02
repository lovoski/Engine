#pragma once

#include "Lights.hpp"
#include "ecs/ecs.hpp"
#include "resource/ResourceManager.hpp"
#include "resource/ResourceTypes.hpp"
#include "resource/Shader.hpp"
#include "resource/MaterialData.hpp"

#include "engine/Engine.hpp"

class BaseMaterial : public ECS::BaseComponent {
public:
  BaseMaterial() {
    SetMaterialData();
  }
  ~BaseMaterial() {}

  void SetMaterialData(string path="::base") {
    matData = Core.RManager.GetMaterialData(path);
  }
  Resource::Shader *GetShader() {
    if (matData == nullptr || matData->shader == nullptr)
      Console.Log("[error]: material has no valid shader\n");
    return matData->shader;
  }
  MaterialData *GetMaterialData() {
    if (matData == nullptr)
      Console.Log("[error]: material has no valid material data\n");
    return matData;
  }

  // pass variables and textures to shaders
  void SetBaseVariables() {
    setFixedVariables();
    activateTextures();
  }

  void SetBaseLights(vector<BaseLight> &lights) {
    unsigned int dirLightCounter = 0;
    unsigned int pointLightCounter = 0;
    unsigned int spotLightCounter = 0;
    // set the properties of different lights
    for (auto &light : lights) {
      if (light.Type == BaseLight::DIRECTIONAL_LIGHT) {
        string lightDirName = "dLightDir" + std::to_string(dirLightCounter);
        string lightColorName = "dLightColor" + std::to_string(dirLightCounter);
        matData->shader->SetVec3(lightDirName, ECS::EManager.EntityFromID(light.GetID())->LocalForward);
        matData->shader->SetVec3(lightColorName, light.LightColor);
        dirLightCounter++;
      } else if (light.Type == BaseLight::POINT_LIGHT) {
        string lightPosName = "pLightPos" + std::to_string(pointLightCounter);
        string lightColorName =
            "pLightColor" + std::to_string(pointLightCounter);
        matData->shader->SetVec3(
            lightPosName,
            ECS::EManager.EntityFromID(light.GetID())->Position());
        matData->shader->SetVec3(lightColorName, light.LightColor);
        pointLightCounter++;
      } else if (light.Type == BaseLight::SPOT_LIGHT) {
      }
    }
    if (dirLightCounter == 0) {
      Console.Log("[error]: At least one directional light needed for the default shader\n");
    }
  }

  MaterialData *matData = nullptr;
private:

  // if there's any changes in these properties, these variables must be updated
  void setFixedVariables() {
    for (auto intVar : matData->intVariables)
      matData->shader->SetInt(matData->variableNames[intVar.first], intVar.second);
    for (auto floatVar : matData->floatVariables)
      matData->shader->SetFloat(matData->variableNames[floatVar.first], floatVar.second);
    for (auto vec2Var : matData->vec2Variables)
      matData->shader->SetVec2(matData->variableNames[vec2Var.first], vec2Var.second);
    for (auto vec3Var : matData->vec3Variables)
      matData->shader->SetVec3(matData->variableNames[vec3Var.first], vec3Var.second);
    for (auto vec4Var : matData->vec4Variables)
      matData->shader->SetVec4(matData->variableNames[vec4Var.first], vec4Var.second);
  }

  void activateTextures() {
    unsigned int texCounter = 0;
    for (auto tex : matData->texVariables) {
      if (tex.second->type != Resource::TEXTURE_TYPE::ICON_TEXTURE) { // don't render icon texture
        glActiveTexture(GL_TEXTURE0 +
                        texCounter); // active proper texture unit before binding
        // retrieve texture number (the N in diffuse_textureN)
        string name = matData->variableNames[tex.first];
        // now set the sampler to the correct texture unit
        glUniform1i(glGetUniformLocation(matData->shader->ID, name.c_str()), texCounter);
        // and finally bind the texture
        glBindTexture(GL_TEXTURE_2D, tex.second->id);
        texCounter++;
      }
    }
  }
};