#include "MaterialData.hpp"

void MaterialData::SetDefaultMaterial() {
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