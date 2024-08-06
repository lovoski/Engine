#pragma once

#include "global.hpp"
#include "resource/ResourceTypes.hpp"
#include "resource/Shader.hpp"


#include "ecs/components/Lights.hpp"

// Each material data should maintain an instance shader
// If the shader specified failed to load, a default error shader should be
// applied The material data should be able to represent a complete render pass
class MaterialData {
public:
  MaterialData() {}
  ~MaterialData() {
    if (shader != nullptr)
      delete shader;
  }

  string identifier;
  string path;

  string vertShaderPath, fragShaderPath, geomShaderPath;

  std::map<int, string> variableNames;
  std::map<int, int> intVariables;
  std::map<int, float> floatVariables;
  std::map<int, vec2> vec2Variables;
  std::map<int, vec3> vec3Variables;
  std::map<int, vec4> vec4Variables;
  // textures
  std::map<int, Resource::Texture *> texVariables;

  std::map<int, std::pair<float, float>> floatVariablesRange;

  // Set to diffuse material by default
  void SetDiffuseMaterial();
  // Set to pbr material by default
  void SetPBRMaterial();
  // force reload, compile and link the shader program
  void LoadShader(string vsp = Resource::DiffuseShaderPath + ".vert",
                  string fsp = Resource::DiffuseShaderPath + ".frag",
                  string gsp = "none");

  Resource::Shader *GetShader() { return shader; }

  template <typename T> bool AddVariable(string name, T value);
  template <typename T> bool RemoveVariable(string name);

  template <typename T> bool SetVariableRange(string name, T min, T max);

  void Serialize(Json &json);
  void Deserialize(Json &json);

  void SetupLights(vector<Light> &lights);
  void SetupVariables();

private:
  int idCounter = 0;

  Resource::Shader *shader = nullptr;

  bool HasNamingConflict(string name);

  void setFixedVariables();
  void activateTextures();
};

// template functions must be fully defined in the header file
template <typename T>
bool MaterialData::AddVariable(std::string name, T value) {
  if (HasNamingConflict(name)) {
    return false;
  }
  variableNames[idCounter] = name;
  if constexpr (std::is_same_v<T, int>) {
    intVariables[idCounter] = value;
  } else if constexpr (std::is_same_v<T, float>) {
    floatVariables[idCounter] = value;
  } else if constexpr (std::is_same_v<T, vec2>) {
    vec2Variables[idCounter] = value;
  } else if constexpr (std::is_same_v<T, vec3>) {
    vec3Variables[idCounter] = value;
  } else if constexpr (std::is_same_v<T, vec4>) {
    vec4Variables[idCounter] = value;
  } else if constexpr (std::is_same_v<T, Resource::Texture *>) {
    texVariables[idCounter] = value;
  } else {
    std::cout << "Setting unsupported type to material data, type: "
              << typeid(T).name() << std::endl;
    return false;
  }
  ++idCounter;
  return true;
}

template <typename T> bool MaterialData::RemoveVariable(string name) {
  int idTBR = -1;
  for (auto ele : variableNames)
    if (ele.second == name)
      idTBR = ele.first;
  if (idTBR == -1) {
    cout << "variable " << name << " not found in material data" << endl;
    return false;
  }
  variableNames.erase(idTBR);
  if constexpr (std::is_same_v<T, int>) {
    intVariables.erase(idTBR);
  } else if constexpr (std::is_same_v<T, float>) {
    floatVariables.erase(idTBR);
  } else if constexpr (std::is_same_v<T, vec2>) {
    vec2Variables.erase(idTBR);
  } else if constexpr (std::is_same_v<T, vec3>) {
    vec3Variables.erase(idTBR);
  } else if constexpr (std::is_same_v<T, vec4>) {
    vec4Variables.erase(idTBR);
  } else if constexpr (std::is_same_v<T, Resource::Texture *>) {
    texVariables.erase(idTBR);
  } else {
    cout << "removing unsupported type to material data, type: "
         << typeid(T).name() << endl;
    return false;
  }
  return true;
}

template <typename T>
bool MaterialData::SetVariableRange(string name, T min, T max) {
  int id = -1;
  for (auto ele : variableNames)
    if (ele.second == name)
      id = ele.first;
  if (id == -1) {
    cout << "variable " << name << " not found in material data" << endl;
    return false;
  }
  if constexpr (std::is_same_v<T, float>) {
    floatVariablesRange[id] = std::make_pair(min, max);
  } else {
    cout << "type " << typeid(T).name() << " don't support range specification"
         << endl;
    return false;
  }
  return true;
}