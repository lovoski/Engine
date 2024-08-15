#include "Component/Material.hpp"

namespace aEngine {

void Material::DrawInspectorGUI() {
  if (ImGui::TreeNode("Material")) {
    const ImVec2 sizeAvailable = ImGui::GetContentRegionAvail();
    ImGui::BeginChild("BaseMaterialEditor", {-1, sizeAvailable.y});
    ImGui::MenuItem(("Material: " + passes[0]->identifier).c_str(), nullptr,
                    nullptr, false);
    if (ImGui::Button("Save", {-1, 30})) {
      std::ofstream materialOut(passes[0]->path);
      if (!materialOut.is_open()) {
        Console.Log("[error]: can't save material to %s\n",
                    passes[0]->path.c_str());
      } else {
        Json output;
        passes[0]->Serialize(output);
        materialOut << output;
        Console.Log("[info]: save material to %s\n", passes[0]->path.c_str());
      }
      materialOut.close();
    }
    // shader information
    ImGui::BeginGroup();
    ImGui::MenuItem("Shaders", nullptr, nullptr, false);
    if (ImGui::Button("Reset", {sizeAvailable.x / 2, 30})) {
      passes[0]->LoadShader(); // reset to defualt shader
      Console.Log("[info]: reset to default shaders\n");
    }
    ImGui::SameLine();
    if (ImGui::Button("Reload", {-1, 30})) {
      passes[0]->LoadShader(passes[0]->vertShaderPath,
                            passes[0]->fragShaderPath,
                            passes[0]->geomShaderPath);
      Console.Log("[info]: reload shader %s, %s, %s\n",
                  passes[0]->vertShaderPath.c_str(),
                  passes[0]->fragShaderPath.c_str(),
                  passes[0]->geomShaderPath.c_str());
    }
    ImGui::TextWrapped("Vertex Shader: %s", passes[0]->vertShaderPath.c_str());
    ImGui::TextWrapped("Fragment Shader: %s",
                       passes[0]->fragShaderPath.c_str());
    ImGui::TextWrapped("Geometry Shader: %s",
                       passes[0]->geomShaderPath.c_str());
    ImGui::EndGroup();
    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload =
              ImGui::AcceptDragDropPayload("SHADER_OVERRIDE")) {
        char *info = (char *)payload->Data;
        std::string base =
            std::string(info).substr(0, std::string(info).find_last_of('.'));
        passes[0]->LoadShader(base + ".vert", base + ".frag");
      }
      ImGui::EndDragDropTarget();
    }
    ImGui::MenuItem("Material Variables", nullptr, nullptr, false);
    // get the pointer
    auto ptr = passes[0];
    // construct dynamic material editor from the maps
    if (ImGui::Button("New Variable", {-1, 30})) {
      ImGui::OpenPopup("AddNewVariable");
    }
    if (ImGui::BeginPopup("AddNewVariable")) {
      static char newVariableName[100];
      static int selectedNewVariableType = 0;
      const char *newVariableTypes[] = {"int",  "float", "vec2",
                                        "vec3", "vec4",  "texture"};
      ImGui::MenuItem("Variable Name", nullptr, nullptr, false);
      ImGui::PushItemWidth(200);
      ImGui::InputText("##newmaterialvariablename", newVariableName,
                       sizeof(newVariableName));
      ImGui::Combo("Type", &selectedNewVariableType, newVariableTypes, 6);
      // float
      static float minAndMax[2] = {0.0f, 1.0f};
      if (selectedNewVariableType == 1) {
        ImGui::InputFloat2("Min & Max", minAndMax);
      }
      if (ImGui::Button("Create", {-1, 30})) {
        if (strlen(newVariableName) == 0) {
          Console.Log("[error]: variable name can't be null\n");
        } else {
          if (selectedNewVariableType == 0) {
            ptr->AddVariable(newVariableName, 0);
          } else if (selectedNewVariableType == 1) {
            ptr->AddVariable(newVariableName, 0.0f);
            ptr->SetVariableRange(newVariableName, minAndMax[0], minAndMax[1]);
          } else if (selectedNewVariableType == 2) {
            ptr->AddVariable(newVariableName, glm::vec2(0.0f));
          } else if (selectedNewVariableType == 3) {
            ptr->AddVariable(newVariableName, glm::vec3(0.0f));
          } else if (selectedNewVariableType == 4) {
            ptr->AddVariable(newVariableName, glm::vec4(0.0f));
          } else if (selectedNewVariableType == 5) {
            ptr->AddVariable(newVariableName, Loader.GetTexture("::NULL_ICON"));
          }
          std::strcpy(newVariableName, "");
        }
        ImGui::CloseCurrentPopup(); // close the popup after creating one
                                    // variable
      }
      ImGui::PopItemWidth();
      ImGui::EndPopup();
    }
    // int variables
    if (ptr->intVariables.size() > 0) {
      // don't modify the map when iterating through it
      std::vector<std::string> toRemove;
      ImGui::MenuItem("Int Variables", nullptr, nullptr, false);
      for (auto &intVar : ptr->intVariables) {
        std::string name = ptr->variableNames[intVar.first];
        if (ImGui::Button(("X##" + name).c_str()))
          toRemove.push_back(name);
        ImGui::SameLine();
        ImGui::InputInt(name.c_str(), &intVar.second);
      }
      for (auto name : toRemove)
        ptr->RemoveVariable<int>(name);
    }
    // float variables
    if (ptr->floatVariables.size() > 0) {
      std::vector<std::string> toRemove;
      ImGui::MenuItem("Float Variables", nullptr, nullptr, false);
      for (auto &floatVar : ptr->floatVariables) {
        float minRange = -10000.0f, maxRange = 10000.0f;
        if (ptr->floatVariablesRange.find(floatVar.first) !=
            ptr->floatVariablesRange.end()) {
          auto range = ptr->floatVariablesRange[floatVar.first];
          minRange = range.first;
          maxRange = range.second;
        }
        std::string name = ptr->variableNames[floatVar.first];
        if (ImGui::Button(("X##" + name).c_str()))
          toRemove.push_back(name);
        ImGui::SameLine();
        ImGui::SliderFloat(name.c_str(), &floatVar.second, minRange, maxRange);
      }
      for (auto name : toRemove)
        ptr->RemoveVariable<float>(name);
    }
    // vec2 variable
    if (ptr->vec2Variables.size() > 0) {
      std::vector<std::string> toRemove;
      ImGui::MenuItem("Vec2 Variables", nullptr, nullptr, false);
      for (auto &vec2Var : ptr->vec2Variables) {
        std::string name = ptr->variableNames[vec2Var.first];
        float vec2Value[2] = {vec2Var.second.x, vec2Var.second.y};
        if (ImGui::Button(("X##" + name).c_str()))
          toRemove.push_back(name);
        ImGui::SameLine();
        if (ImGui::DragFloat2(name.c_str(), vec2Value))
          ptr->vec2Variables[vec2Var.first] =
              glm::vec2(vec2Value[0], vec2Value[1]);
      }
      for (auto name : toRemove)
        ptr->RemoveVariable<glm::vec2>(name);
    }
    // vec3 variable
    if (ptr->vec3Variables.size() > 0) {
      std::vector<std::string> toRemove;
      ImGui::MenuItem("Vec3 Variables", nullptr, nullptr, false);
      for (auto &vec3Var : ptr->vec3Variables) {
        std::string name = ptr->variableNames[vec3Var.first];
        float vec3Value[3] = {vec3Var.second.x, vec3Var.second.y,
                              vec3Var.second.z};
        if (ImGui::Button(("X##" + name).c_str()))
          toRemove.push_back(name);
        ImGui::SameLine();
        if (name == "Albedo" || name == "Specular") {
          if (ImGui::ColorEdit3(name.c_str(), vec3Value))
            ptr->vec3Variables[vec3Var.first] =
                glm::vec3(vec3Value[0], vec3Value[1], vec3Value[2]);
        } else {
          if (ImGui::DragFloat3(name.c_str(), vec3Value, 0.01f))
            ptr->vec3Variables[vec3Var.first] =
                glm::vec3(vec3Value[0], vec3Value[1], vec3Value[2]);
        }
      }
      for (auto name : toRemove)
        ptr->RemoveVariable<glm::vec3>(name);
    }
    // vec4 variable
    if (ptr->vec4Variables.size() > 0) {
      std::vector<std::string> toRemove;
      ImGui::MenuItem("Vec4 Variables", nullptr, nullptr, false);
      for (auto &vec4Var : ptr->vec4Variables) {
        std::string name = ptr->variableNames[vec4Var.first];
        float vec4Value[4] = {vec4Var.second.x, vec4Var.second.y,
                              vec4Var.second.z, vec4Var.second.w};
        if (ImGui::Button(("X##" + name).c_str()))
          toRemove.push_back(name);
        ImGui::SameLine();
        if (name == "Albedo" || name == "Specular") {
          if (ImGui::ColorEdit4(name.c_str(), vec4Value))
            ptr->vec4Variables[vec4Var.first] = glm::vec4(
                vec4Value[0], vec4Value[1], vec4Value[2], vec4Value[3]);
        } else {
          if (ImGui::DragFloat4(name.c_str(), vec4Value, 0.01f))
            ptr->vec4Variables[vec4Var.first] = glm::vec4(
                vec4Value[0], vec4Value[1], vec4Value[2], vec4Value[3]);
        }
      }
      for (auto name : toRemove)
        ptr->RemoveVariable<glm::vec4>(name);
    }
    // texture variable
    if (ptr->texVariables.size() > 0) {
      ImGui::MenuItem("Texture Variable", nullptr, nullptr, false);
      const int textureSize = 128;
      std::vector<std::string> toRemove;
      for (auto &texVar : ptr->texVariables) {
        std::string name = ptr->variableNames[texVar.first];
        if (ImGui::Button(("X##" + name).c_str()))
          toRemove.push_back(name);
        ImGui::SameLine();
        // the pointer to texture is a texture slot
        ImGui::Image((void *)texVar.second->id, {textureSize, textureSize});
        // // all texture can be replaced with another texture
        if (ImGui::BeginDragDropTarget()) {
          if (const ImGuiPayload *payload =
                  ImGui::AcceptDragDropPayload("LOAD_TEXTURE")) {
            char *texturePath = (char *)payload->Data;
            auto newTexture = Loader.GetTexture(texturePath);
            // replace null texture with the actual texture
            texVar.second = newTexture; // replace upload texture with actual
                                        // loaded texture
          }
          ImGui::EndDragDropTarget();
        }
        ImGui::SameLine();
        ImGui::Text(name.c_str());
      }
      for (auto name : toRemove)
        ptr->RemoveVariable<Texture *>(name);
    }
    ImGui::EndChild();
    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload =
              ImGui::AcceptDragDropPayload("MATERIAL_OVERRIDE")) {
        char *info = (char *)payload->Data;
        // Console.Log(info);
        auto newPass = Loader.GetMaterial(info);
        if (newPass != nullptr) // setup new pass if the pointer is not null
          passes[0] = Loader.GetMaterial(info);
        // GWORLD.EntityFromID(entity->ID)->AssignChild(newChild);
      }
      ImGui::EndDragDropTarget();
    }
    ImGui::TreePop();
  }
}

}; // namespace aEngine