#include <limits>
#include "../Editor.hpp"

#include "Component/Camera.hpp"
#include "Component/Material.hpp"
#include "Component/MeshRenderer.hpp"
#include "Component/Light.hpp"

#include "Utils/Render/Shader.hpp"
#include "Utils/Render/MaterialData.hpp"

using glm::vec2;
using glm::vec3;
using glm::vec4;
using std::string;
using std::vector;

#define MAX_FLOAT std::numeric_limits<float>::max()

inline void DrawTransformGUI(EntityID selectedEntity, Engine *engine) {
  auto transform = GWORLD.EntityFromID(selectedEntity);
  if (ImGui::TreeNode("Transform")) {
    ImGui::MenuItem("Global Properties", nullptr, nullptr, false);
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
    if (ImGui::DragFloat3("Rotation", rotations, 1.0f, -180.0f, 180.0f))
      ;
    transform->SetGlobalRotationDegree(
        vec3(rotations[0], rotations[1], rotations[2]));
    if (ImGui::DragFloat3("Scale", scales, 0.01f, 0.0f, MAX_FLOAT))
      ;
    transform->SetGlobalScale(vec3(scales[0], scales[1], scales[2]));
    ImGui::TreePop();
  }
}

inline void DrawCameraGUI(EntityID selectedEntity, Engine *engine) {
  auto &camera = GWORLD.GetComponent<Camera>(selectedEntity);
  if (ImGui::TreeNode("Camera")) {
    ImGui::DragFloat(" :Fov  Y", &camera.fovY, 1.0f, 0.0f, 150.0f);
    ImGui::DragFloat(" :Z Near", &camera.zNear, 0.001f, 0.0000001f, 10.0f);
    ImGui::DragFloat(" :Z  Far", &camera.zFar, 0.1f, 20.0f, 2000.0f);
    ImGui::TreePop();
  }
}

inline void DrawBaseMaterialGUI(EntityID selectedEntity, Engine *engine) {
  auto &material = GWORLD.GetComponent<Material>(selectedEntity);
  if (ImGui::TreeNode("Material")) {
    const ImVec2 sizeAvailable = ImGui::GetContentRegionAvail();
    ImGui::BeginChild("BaseMaterialEditor", {-1, sizeAvailable.y});
    ImGui::MenuItem(("Material: " + material.passes[0]->identifier).c_str(),
                    nullptr, nullptr, false);
    if (ImGui::Button("Save", {-1, 30})) {
      std::ofstream materialOut(material.passes[0]->path);
      if (!materialOut.is_open()) {
        Console.Log("[error]: can't save material to %s\n",
                    material.passes[0]->path.c_str());
      } else {
        Json output;
        material.passes[0]->Serialize(output);
        materialOut << output;
        Console.Log("[info]: save material to %s\n",
                    material.passes[0]->path.c_str());
      }
      materialOut.close();
    }
    // shader information
    ImGui::BeginGroup();
    ImGui::MenuItem("Shaders", nullptr, nullptr, false);
    if (ImGui::Button("Reset", {sizeAvailable.x / 2, 30})) {
      material.passes[0]->LoadShader(); // reset to defualt shader
      Console.Log("[info]: reset to default shaders\n");
    }
    ImGui::SameLine();
    if (ImGui::Button("Reload", {-1, 30})) {
      material.passes[0]->LoadShader(material.passes[0]->vertShaderPath,
                                     material.passes[0]->fragShaderPath,
                                     material.passes[0]->geomShaderPath);
      Console.Log("[info]: reload shader %s, %s, %s\n",
                  material.passes[0]->vertShaderPath.c_str(),
                  material.passes[0]->fragShaderPath.c_str(),
                  material.passes[0]->geomShaderPath.c_str());
    }
    ImGui::TextWrapped("Vertex Shader: %s",
                       material.passes[0]->vertShaderPath.c_str());
    ImGui::TextWrapped("Fragment Shader: %s",
                       material.passes[0]->fragShaderPath.c_str());
    ImGui::TextWrapped("Geometry Shader: %s",
                       material.passes[0]->geomShaderPath.c_str());
    ImGui::EndGroup();
    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload =
              ImGui::AcceptDragDropPayload("SHADER_OVERRIDE")) {
        char *info = (char *)payload->Data;
        string base = string(info).substr(0, string(info).find_last_of('.'));
        material.passes[0]->LoadShader(base + ".vert", base + ".frag");
      }
      ImGui::EndDragDropTarget();
    }
    ImGui::MenuItem("Material Variables", nullptr, nullptr, false);
    // get the pointer
    auto ptr = material.passes[0];
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
            ptr->AddVariable(newVariableName, vec2(0.0f));
          } else if (selectedNewVariableType == 3) {
            ptr->AddVariable(newVariableName, vec3(0.0f));
          } else if (selectedNewVariableType == 4) {
            ptr->AddVariable(newVariableName, vec4(0.0f));
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
      vector<string> toRemove;
      ImGui::MenuItem("Int Variables", nullptr, nullptr, false);
      for (auto &intVar : ptr->intVariables) {
        string name = ptr->variableNames[intVar.first];
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
      vector<string> toRemove;
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
          toRemove.push_back(name);
        ImGui::SameLine();
        ImGui::SliderFloat(name.c_str(), &floatVar.second, minRange, maxRange);
      }
      for (auto name : toRemove)
        ptr->RemoveVariable<float>(name);
    }
    // vec2 variable
    if (ptr->vec2Variables.size() > 0) {
      vector<string> toRemove;
      ImGui::MenuItem("Vec2 Variables", nullptr, nullptr, false);
      for (auto &vec2Var : ptr->vec2Variables) {
        string name = ptr->variableNames[vec2Var.first];
        float vec2Value[2] = {vec2Var.second.x, vec2Var.second.y};
        if (ImGui::Button(("X##" + name).c_str()))
          toRemove.push_back(name);
        ImGui::SameLine();
        if (ImGui::DragFloat2(name.c_str(), vec2Value))
          ptr->vec2Variables[vec2Var.first] = vec2(vec2Value[0], vec2Value[1]);
      }
      for (auto name : toRemove)
        ptr->RemoveVariable<vec2>(name);
    }
    // vec3 variable
    if (ptr->vec3Variables.size() > 0) {
      vector<string> toRemove;
      ImGui::MenuItem("Vec3 Variables", nullptr, nullptr, false);
      for (auto &vec3Var : ptr->vec3Variables) {
        string name = ptr->variableNames[vec3Var.first];
        float vec3Value[3] = {vec3Var.second.x, vec3Var.second.y,
                              vec3Var.second.z};
        if (ImGui::Button(("X##" + name).c_str()))
          toRemove.push_back(name);
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
      for (auto name : toRemove)
        ptr->RemoveVariable<vec3>(name);
    }
    // vec4 variable
    if (ptr->vec4Variables.size() > 0) {
      vector<string> toRemove;
      ImGui::MenuItem("Vec4 Variables", nullptr, nullptr, false);
      for (auto &vec4Var : ptr->vec4Variables) {
        string name = ptr->variableNames[vec4Var.first];
        float vec4Value[4] = {vec4Var.second.x, vec4Var.second.y,
                              vec4Var.second.z, vec4Var.second.w};
        if (ImGui::Button(("X##" + name).c_str()))
          toRemove.push_back(name);
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
      for (auto name : toRemove)
        ptr->RemoveVariable<vec4>(name);
    }
    // texture variable
    if (ptr->texVariables.size() > 0) {
      ImGui::MenuItem("Texture Variable", nullptr, nullptr, false);
      const int textureSize = 128;
      vector<string> toRemove;
      for (auto &texVar : ptr->texVariables) {
        string name = ptr->variableNames[texVar.first];
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
            texVar.second =
                newTexture; // replace upload texture with actual loaded texture
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
          material.passes[0] = Loader.GetMaterial(info);
        // GWORLD.EntityFromID(entity->ID)->AssignChild(newChild);
      }
      ImGui::EndDragDropTarget();
    }
    ImGui::TreePop();
  }
}

inline void DrawBaseLightGUI(EntityID selectedEntity, Engine *engine) {
  auto &light = GWORLD.GetComponent<Light>(selectedEntity);
  if (ImGui::TreeNode("Base Light")) {
    const char *comboItems[] = {"Directional light", "Point light",
                                "Spot light"};
    static int baseLightGUIComboItemIndex = 0;
    ImGui::Combo("Light Type", &baseLightGUIComboItemIndex, comboItems, 3);
    float lightColor[3] = {light.lightColor.x, light.lightColor.y,
                           light.lightColor.z};
    if (baseLightGUIComboItemIndex == 0) {
      ImGui::ColorEdit3("Light Color", lightColor);
    } else if (baseLightGUIComboItemIndex == 1) {
    } else if (baseLightGUIComboItemIndex == 2) {
    }
    light.lightColor.x = lightColor[0];
    light.lightColor.y = lightColor[1];
    light.lightColor.z = lightColor[2];
    ImGui::TreePop();
  }
}

void Editor::InspectorWindow() {
  ImGui::Begin("Components");
  // // Right-click context menu for the parent window
  // if (!ImGui::IsAnyItemHovered() &&
  //     ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) {
  //   // open the window context menu
  //   if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
  //     ImGui::OpenPopup("ComponentWindowContextMenu");
  //   // unselect entities
  //   // if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
  //   //   selectedEntity = (ECS::EntityID)(-1);
  // }
  // if (selectedEntity != (ECS::EntityID)(-1) &&
  // ImGui::BeginPopup("ComponentWindowContextMenu")) {
  //   ImGui::MenuItem("Window Options", nullptr, nullptr, false);
  //   ImGui::EndPopup();
  // }
  if (context.selectedEntity != (EntityID)(-1)) {
    string entityName =
        "Active Entity : " + GWORLD.EntityFromID(context.selectedEntity)->name;
    ImGui::SeparatorText(entityName.c_str());
    ImGui::BeginChild("Components List",
                      {-1, ImGui::GetContentRegionAvail().y});
    DrawTransformGUI(context.selectedEntity, engine);
    if (GWORLD.HasComponent<Camera>(context.selectedEntity))
      DrawCameraGUI(context.selectedEntity, engine);
    if (GWORLD.HasComponent<Material>(context.selectedEntity))
      DrawBaseMaterialGUI(context.selectedEntity, engine);
    if (GWORLD.HasComponent<Light>(context.selectedEntity))
      DrawBaseLightGUI(context.selectedEntity, engine);
    ImGui::EndChild();
  }
  ImGui::End();
}
