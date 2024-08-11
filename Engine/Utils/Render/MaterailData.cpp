#include "Utils/Render/MaterialData.hpp"
#include "Utils/AssetsLoader.hpp"
#include "Scene.hpp"

namespace aEngine {

namespace Render {

using glm::vec3;
using glm::vec2;
using std::string;
using std::vector;

void MaterialData::SetDiffuseMaterial() {
  AddVariable("Albedo", vec3(1.0f));
  AddVariable("Specular", vec3(1.0f));
  AddVariable("Ambient", 0.1f);
  SetVariableRange("Ambient", 0.0f, 1.0f);
  AddVariable("Smootheness", 32);
}

bool MaterialData::HasNamingConflict(string name) {
  for (auto ele : variableNames) {
    if (ele.second == name)
      return true;
  }
  return false;
}

void MaterialData::Serialize(Json &json) {
  json["variableNames"] = variableNames;
  json["intVariables"] = intVariables;
  json["floatVariables"] = floatVariables;
  json["vec2Variables"] = vec2Variables;
  json["vec3Variables"] = vec3Variables;
  json["vec4Variables"] = vec4Variables;
  json["floatVariablesRange"] = floatVariablesRange;
  // there could be some slots to available textures
  // when a texture is set to be type ICON_TEXTURE, it will 
  // be displayed as some icons on the gui side
  vector<string> texturePathes;
  vector<int> textureIndex;
  for (auto tex : texVariables) {
    textureIndex.push_back(tex.first);
    texturePathes.push_back(tex.second->path);
  }
  json["texVariables"]["indices"] = textureIndex;
  json["texVariables"]["pathes"] = texturePathes;
  json["vsp"] = vertShaderPath;
  json["fsp"] = fragShaderPath;
  json["gsp"] = geomShaderPath;
  json["idCounter"] = idCounter;
}

void MaterialData::Deserialize(Json &json) {
  variableNames = json["variableNames"];
  intVariables = json["intVariables"];
  floatVariables = json["floatVariables"];
  vec2Variables = json["vec2Variables"];
  vec3Variables = json["vec3Variables"];
  vec4Variables = json["vec4Variables"];
  floatVariablesRange = json["floatVariablesRange"];
  vector<int> texIndices = json["texVariables"]["indices"];
  vector<string> texPathes = json["texVariables"]["pathes"];
  for (int i = 0; i < texIndices.size(); ++i) {
    texVariables.insert(
          std::make_pair(texIndices[i], Loader.GetTexture(texPathes[i])));
  }
  // load the shader if possible
  LoadShader(json["vsp"], json["fsp"], json["gsp"]);
  // reset id counter
  idCounter = json["idCounter"].get<int>();
}

void MaterialData::SetupLights(vector<Light> &lights) {
  unsigned int dirLightCounter = 0;
  unsigned int pointLightCounter = 0;
  unsigned int spotLightCounter = 0;
  // set the properties of different lights
  for (auto &light : lights) {
    if (light.type == LIGHT_TYPE::DIRECTIONAL_LIGHT) {
      string lightDirName = "dLightDir" + std::to_string(dirLightCounter);
      string lightColorName = "dLightColor" + std::to_string(dirLightCounter);
      shader->SetVec3(lightDirName, GWORLD.EntityFromID(light.GetID())->LocalForward);
      shader->SetVec3(lightColorName, light.lightColor);
      dirLightCounter++;
    } else if (light.type == LIGHT_TYPE::POINT_LIGHT) {
      string lightPosName = "pLightPos" + std::to_string(pointLightCounter);
      string lightColorName =
          "pLightColor" + std::to_string(pointLightCounter);
      shader->SetVec3(
          lightPosName,
          GWORLD.EntityFromID(light.GetID())->Position());
      shader->SetVec3(lightColorName, light.lightColor);
      pointLightCounter++;
    } else if (light.type == LIGHT_TYPE::SPOT_LIGHT) {
    }
  }
  if (dirLightCounter == 0) {
    Console.Log("[error]: At least one directional light needed for the default shader\n");
  }
}

void MaterialData::SetupVariables() {
  setFixedVariables();
  activateTextures();
}

void MaterialData::setFixedVariables() {
  shader->Use();
  for (auto intVar : intVariables)
    shader->SetInt(variableNames[intVar.first], intVar.second);
  for (auto floatVar : floatVariables)
    shader->SetFloat(variableNames[floatVar.first], floatVar.second);
  for (auto vec2Var : vec2Variables)
    shader->SetVec2(variableNames[vec2Var.first], vec2Var.second);
  for (auto vec3Var : vec3Variables)
    shader->SetVec3(variableNames[vec3Var.first], vec3Var.second);
  for (auto vec4Var : vec4Variables)
    shader->SetVec4(variableNames[vec4Var.first], vec4Var.second);
}

void MaterialData::activateTextures() {
  shader->Use();
  unsigned int texCounter = 0;
  for (auto tex : texVariables) {
    // don't pass the texture to gpu if its a texture slot
    if (tex.second->path != "::textureSlot") {
      if (texCounter >= GL_MAX_TEXTURE_UNITS) {
        Console.Log("[error]: exceeded maximum texture numbers\n");
        break;
      }
       // active proper texture unit before binding
      glActiveTexture(GL_TEXTURE0 + texCounter);
      string name = variableNames[tex.first];
      int location = glGetUniformLocation(shader->ID, name.c_str());
      if (location == -1) {
        // Console.Log("[warning]: uniform %s not found in shader\n", name.c_str());
        continue;
      }
      glUniform1i(location, texCounter);
      glBindTexture(GL_TEXTURE_2D, tex.second->id);
      texCounter++;
    }
  }
}

void MaterialData::LoadShader(string vsp, string fsp, string gsp) {
  if (shader == nullptr)
    shader = new Render::Shader();
  if (!shader->LoadAndRecompileShader(vsp, fsp, gsp)) {
    // set shader to error shader if the load, compile and link falied
    shader->LoadAndRecompileShaderSource(errorVS, errorFS);
    vertShaderPath = "Built in error shader";
    fragShaderPath = "Built in error shader";
    geomShaderPath = "none";
  } else {
    vertShaderPath = vsp;
    fragShaderPath = fsp;
    geomShaderPath = gsp;
  }
}

};

};