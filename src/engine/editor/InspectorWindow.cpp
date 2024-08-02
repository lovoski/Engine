#include "engine/EditorWindows.hpp"

inline void DrawTransformGUI(ECS::EntityID selectedEntity) {
  auto transform = ECS::EManager.EntityFromID(selectedEntity);
  if (ImGui::TreeNode("Transform")) {
    ImGui::SeparatorText("Global Properties");
    vec3 position = transform->Position();
    vec3 scale = transform->Scale();
    vec3 angles = transform->EulerAnglesDegree();
    float positions[3] = {position.x, position.y, position.z};
    float scales[3] = {scale.x, scale.y, scale.z};
    float rotations[3] = {angles.x, angles.y, angles.z};
    if (ImGui::DragFloat3("Position", positions, 0.01f, -MAX_FLOAT, MAX_FLOAT))
      ;
    transform->SetGlobalPosition(
        vec3(positions[0], positions[1], positions[2]));
    ImGui::Separator();
    if (ImGui::DragFloat3("Rotation", rotations, 1.0f, -180.0f, 180.0f))
      ;
    transform->SetGlobalRotationDegree(
        vec3(rotations[0], rotations[1], rotations[2]));
    ImGui::Separator();
    if (ImGui::DragFloat3("Scale", scales, 0.01f, 0.0f, MAX_FLOAT))
      ;
    transform->SetGlobalScale(vec3(scales[0], scales[1], scales[2]));
    ImGui::TreePop();
  }
}

inline void DrawCameraGUI(ECS::EntityID selectedEntity) {
  auto &camera = ECS::EManager.GetComponent<Camera>(selectedEntity);
  if (ImGui::TreeNode("Camera")) {
    ImGui::DragFloat(" :Fov  Y", &camera.fovY, 1.0f, 0.0f, 150.0f);
    ImGui::DragFloat(" :Z Near", &camera.zNear, 0.001f, 0.0000001f, 10.0f);
    ImGui::DragFloat(" :Z  Far", &camera.zFar, 0.1f, 20.0f, 2000.0f);
    ImGui::TreePop();
  }
}

inline void DrawBaseMaterialGUI(ECS::EntityID selectedEntity) {
  auto &material = ECS::EManager.GetComponent<BaseMaterial>(selectedEntity);
  if (ImGui::TreeNode("Base Material")) {
    ImGui::BeginChild("BaseMaterialEditor",
                      {-1, ImGui::GetContentRegionAvail().y});
    ImGui::SeparatorText(
        ("Material Name: " + material.matData->identifier).c_str());
    // shader information
    ImGui::BeginGroup();
    ImGui::MenuItem(
        ("Shaders: " + material.matData->shader->identifier).c_str(), nullptr,
        nullptr, false);
    if (ImGui::Button("Reset")) {
      material.matData->SetShader(); // reset to defualt shader
    }
    ImGui::SameLine();
    if (ImGui::Button("Reload")) {
      material.matData->shader = Core.RManager.GetShader(
          material.matData->shader->vertexShaderPath,
          material.matData->shader->fragShaderPath, "none", true);
    }
    ImGui::TextWrapped("Vertex Shader: %s",
                       material.matData->shader->vertexShaderPath.c_str());
    ImGui::TextWrapped("Fragment Shader: %s",
                       material.matData->shader->fragShaderPath.c_str());
    ImGui::EndGroup();
    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload =
              ImGui::AcceptDragDropPayload("SHADER_OVERRIDE")) {
        char *info = (char *)payload->Data;
        string base = string(info).substr(0, string(info).find_last_of('.'));
        material.matData->shader =
            Core.RManager.GetShader(base + ".vert", base + ".frag");
        // Console.Log("%s\n", fs::path(info).stem().string().c_str());
      }
      ImGui::EndDragDropTarget();
    }
    ImGui::MenuItem("Material Variables", nullptr, nullptr, false);
    // get the pointer
    auto ptr = material.matData;
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
            material.matData->AddVariable(newVariableName, 0);
          } else if (selectedNewVariableType == 1) {
            material.matData->AddVariable(newVariableName, 0.0f);
          } else if (selectedNewVariableType == 2) {
            material.matData->AddVariable(newVariableName, vec2(0.0f));
          } else if (selectedNewVariableType == 3) {
            material.matData->AddVariable(newVariableName, vec3(0.0f));
          } else if (selectedNewVariableType == 4) {
            material.matData->AddVariable(newVariableName, vec4(0.0f));
          } else if (selectedNewVariableType == 5) {
            material.matData->AddVariable(newVariableName,
                                          Core.RManager.TextureSlot);
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
    if (ptr->intVariables.size() > 0)
      ImGui::MenuItem("Int Variables", nullptr, nullptr, false);
    for (auto &intVar : ptr->intVariables) {
      string name = ptr->variableNames[intVar.first];
      if (ImGui::Button(("X##" + name).c_str()))
        ptr->RemoveVariable<int>(name);
      ImGui::SameLine();
      ImGui::InputInt(name.c_str(), &intVar.second);
    }
    // float variables
    if (ptr->floatVariables.size() > 0)
      ImGui::MenuItem("Float Variables", nullptr, nullptr, false);
    for (auto &floatVar : ptr->floatVariables) {
      float minRange = -10000.0f, maxRange = 10000.0f;
      if (ptr->floatVariablesRange.find(floatVar.first) !=
          ptr->floatVariablesRange.end()) {
        auto range = ptr->floatVariablesRange[floatVar.first];
        minRange = range.first;
        maxRange = range.second;
      }
      string name = ptr->variableNames[floatVar.first];
      if (ImGui::Button(("X##" + name).c_str()))
        ptr->RemoveVariable<float>(name);
      ImGui::SameLine();
      ImGui::SliderFloat(name.c_str(), &floatVar.second, minRange, maxRange);
    }
    static char floatVariableName[50];
    // vec2 variable
    if (ptr->vec2Variables.size() > 0)
      ImGui::MenuItem("Vec2 Variables", nullptr, nullptr, false);
    for (auto &vec2Var : ptr->vec2Variables) {
      string name = ptr->variableNames[vec2Var.first];
      float vec2Value[2] = {vec2Var.second.x, vec2Var.second.y};
      if (ImGui::Button(("X##" + name).c_str()))
        ptr->RemoveVariable<vec2>(name);
      ImGui::SameLine();
      if (ImGui::DragFloat2(name.c_str(), vec2Value))
        ptr->vec2Variables[vec2Var.first] = vec2(vec2Value[0], vec2Value[1]);
    }
    static char vec2VariableName[50];
    // vec3 variable
    if (ptr->vec3Variables.size() > 0)
      ImGui::MenuItem("Vec3 Variables", nullptr, nullptr, false);
    for (auto &vec3Var : ptr->vec3Variables) {
      string name = ptr->variableNames[vec3Var.first];
      float vec3Value[3] = {vec3Var.second.x, vec3Var.second.y,
                            vec3Var.second.z};
      if (ImGui::Button(("X##" + name).c_str()))
        ptr->RemoveVariable<vec3>(name);
      ImGui::SameLine();
      if (name == "Albedo" || name == "Specular") {
        if (ImGui::ColorEdit3(name.c_str(), vec3Value))
          ptr->vec3Variables[vec3Var.first] =
              vec3(vec3Value[0], vec3Value[1], vec3Value[2]);
      } else {
        if (ImGui::DragFloat3(name.c_str(), vec3Value, 0.01f))
          ptr->vec3Variables[vec3Var.first] =
              vec3(vec3Value[0], vec3Value[1], vec3Value[2]);
      }
    }
    // vec4 variable
    if (ptr->vec4Variables.size() > 0)
      ImGui::MenuItem("Vec4 Variables", nullptr, nullptr, false);
    for (auto &vec4Var : ptr->vec4Variables) {
      string name = ptr->variableNames[vec4Var.first];
      float vec4Value[4] = {vec4Var.second.x, vec4Var.second.y,
                            vec4Var.second.z, vec4Var.second.w};
      if (ImGui::Button(("X##" + name).c_str()))
        ptr->RemoveVariable<vec4>(name);
      ImGui::SameLine();
      if (name == "Albedo" || name == "Specular") {
        if (ImGui::ColorEdit4(name.c_str(), vec4Value))
          ptr->vec4Variables[vec4Var.first] =
              vec4(vec4Value[0], vec4Value[1], vec4Value[2], vec4Value[3]);
      } else {
        if (ImGui::DragFloat4(name.c_str(), vec4Value, 0.01f))
          ptr->vec4Variables[vec4Var.first] =
              vec4(vec4Value[0], vec4Value[1], vec4Value[2], vec4Value[3]);
      }
    }
    // texture variable
    if (ptr->texVariables.size() > 0)
      ImGui::MenuItem("Texture Variable", nullptr, nullptr, false);
    const int textureSize = 128;
    for (auto &texVar : ptr->texVariables) {
      string name = ptr->variableNames[texVar.first];
      if (ImGui::Button(("X##" + name).c_str()))
        ptr->RemoveVariable<Resource::Texture *>(name);
      ImGui::SameLine();
      // the pointer to texture is a texture slot
      ImGui::Image((void *)texVar.second->id, {textureSize, textureSize});
      // // all texture can be replaced with another texture
      if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload *payload =
                ImGui::AcceptDragDropPayload("LOAD_TEXTURE")) {
          char *texturePath = (char *)payload->Data;
          auto newTexture = Core.RManager.GetTexture(texturePath);
          // replace null texture with the actual texture
          texVar.second =
              newTexture; // replace upload texture with actual loaded texture
        }
        ImGui::EndDragDropTarget();
      }
      ImGui::SameLine();
      ImGui::Text(ptr->variableNames[texVar.first].c_str());
    }
    ImGui::EndChild();
    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload =
              ImGui::AcceptDragDropPayload("MATERIAL_OVERRIDE")) {
        char *info = (char *)payload->Data;
        // Console.Log(info);
        material.matData = Core.RManager.GetMaterialData(info);
        // ECS::EManager.EntityFromID(entity->ID)->AssignChild(newChild);
      }
      ImGui::EndDragDropTarget();
    }
    ImGui::TreePop();
  }
}

inline void DrawBaseLightGUI(ECS::EntityID selectedEntity) {
  auto &light = ECS::EManager.GetComponent<BaseLight>(selectedEntity);
  if (ImGui::TreeNode("Base Light")) {
    const char *comboItems[] = {"Directional light", "Point light",
                                "Spot light"};
    static int baseLightGUIComboItemIndex = 0;
    ImGui::Combo("Light Type", &baseLightGUIComboItemIndex, comboItems, 3);
    float lightColor[3] = {light.LightColor.x, light.LightColor.y,
                           light.LightColor.z};
    if (baseLightGUIComboItemIndex == 0) {
      ImGui::ColorEdit3("Light Color", lightColor);
    } else if (baseLightGUIComboItemIndex == 1) {
    } else if (baseLightGUIComboItemIndex == 2) {
    }
    light.LightColor.x = lightColor[0];
    light.LightColor.y = lightColor[1];
    light.LightColor.z = lightColor[2];
    ImGui::TreePop();
  }
}

void EditorWindows::InspectorWindow() {
  ImGui::Begin("Components");
  if (selectedEntity != (ECS::EntityID)(-1)) {
    string entityName =
        "Active Entity : " + ECS::EManager.EntityFromID(selectedEntity)->name;
    ImGui::SeparatorText(entityName.c_str());
    ImGui::BeginChild("Components List",
                      {-1, ImGui::GetContentRegionAvail().y});
    DrawTransformGUI(selectedEntity);
    if (ECS::EManager.HasComponent<Camera>(selectedEntity))
      DrawCameraGUI(selectedEntity);
    if (ECS::EManager.HasComponent<BaseMaterial>(selectedEntity))
      DrawBaseMaterialGUI(selectedEntity);
    if (ECS::EManager.HasComponent<BaseLight>(selectedEntity))
      DrawBaseLightGUI(selectedEntity);
    ImGui::EndChild();
  }
  ImGui::End();
}
