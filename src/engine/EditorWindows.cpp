#include "EditorWindows.hpp"
#include "resource/MaterialData.hpp"
#include "roboto.h"

void EditorWindows::Initialize() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGui::StyleColorsLight();

  // setup imguizmo
  mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
  mCurrentGizmoMode = ImGuizmo::WORLD;

  io = &ImGui::GetIO();
  io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;

  io->Fonts->AddFontFromMemoryTTF(Roboto_Regular_ttf, Roboto_Regular_ttf_len,
                                  20.0f);

  io->IniFilename = layoutFileName;

  ImGui_ImplGlfw_InitForOpenGL(&Core.Window(), true);
  ImGui_ImplOpenGL3_Init("#version 460");
}

void EditorWindows::Destroy() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void DrawProfiler() {}

void EditorWindows::MainMenuBar() {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      ImGui::MenuItem("Project", nullptr, nullptr, false);
      if (ImGui::MenuItem("Load Scene")) {
      }
      if (ImGui::MenuItem("Save Scene", "CTRL+S")) {
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Window")) {
      if (ImGui::BeginMenu("Gizmos")) {
        ImGui::MenuItem("Axis Space", nullptr, nullptr, false);
        if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
          mCurrentGizmoMode = ImGuizmo::LOCAL;
        ImGui::SameLine();
        if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
          mCurrentGizmoMode = ImGuizmo::WORLD;
        ImGui::Separator();
        ImGui::MenuItem("Grid Options", nullptr, nullptr, false);
        ImGui::Checkbox("Show Grid", &showGizmoGrid);
        ImGui::PushItemWidth(100);
        ImGui::InputInt("Grid Size", &gizmoGridSize);
        ImGui::PopItemWidth();
        ImGui::EndMenu();
      }
      // if (ImGui::MenuItem("Show Profiler")) {
      //   DrawProfiler();
      // }
      // if (ImGui::MenuItem("Show Style Editor")) {
      //   ImGui::ShowStyleEditor();
      // }
      ImGui::EndMenu();
    }
  }
}

inline void DrawHierarchyGUI(Entity *entity, ECS::EntityID &selectedEntity,
                             ImGuiTreeNodeFlags nodeFlag) {
  bool isSelected = selectedEntity == entity->ID;
  ImGuiTreeNodeFlags finalFlag = nodeFlag;
  if (isSelected)
    finalFlag |= ImGuiTreeNodeFlags_Selected;
  if (entity->children.size() == 0)
    finalFlag |= ImGuiTreeNodeFlags_Bullet;
  bool nodeOpen = ImGui::TreeNodeEx((void *)(intptr_t)entity->ID, finalFlag,
                                    entity->name.c_str());
  if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
    selectedEntity = entity->ID;
  // drag drop control
  if (ImGui::BeginDragDropSource()) {
    ImGui::SetDragDropPayload("CHANGE_ENTITY_HIERARCHY", &entity,
                              sizeof(Entity *));
    ImGui::Text("Drag drop to change hierarchy");
    ImGui::EndDragDropSource();
  }
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("CHANGE_ENTITY_HIERARCHY")) {
      Entity *newChild = *(Entity **)payload->Data;
      ECS::EManager.EntityFromID(entity->ID)->AssignChild(newChild);
    }
    ImGui::EndDragDropTarget();
  }
  // right click context menu
  if (ImGui::BeginPopupContextItem(
          (entity->name + std::to_string((unsigned int)entity->ID)).c_str(),
          ImGuiPopupFlags_MouseButtonRight)) {
    ImGui::SeparatorText("Entity Options");
    if (ImGui::MenuItem("Remove")) {
      if (entity->children.size() > 0)
        Console.Log("[info]: Destroy entity %s and all its children\n",
                    entity->name.c_str());
      else
        Console.Log("[info]: Destroy entity %s\n", entity->name.c_str());
      ECS::EManager.DestroyEntity(entity->ID);
      selectedEntity = (ECS::EntityID)(-1);
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }
  if (nodeOpen) {
    for (auto child : entity->children)
      DrawHierarchyGUI(child, selectedEntity, nodeFlag);
    ImGui::TreePop();
  }
}

void EditorWindows::EntitiesWindow() {
  ImGui::Begin("Entities");
  ImGui::SeparatorText("Scene");
  // Right-click context menu for the parent window
  if (!ImGui::IsAnyItemHovered() &&
      ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) {
    // open the window context menu
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
      ImGui::OpenPopup("EntitiesWindowContextMenu");
    // unselect entities
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
      selectedEntity = (ECS::EntityID)(-1);
  }
  if (ImGui::BeginPopup("EntitiesWindowContextMenu")) {
    ImGui::MenuItem("Window Options", nullptr, nullptr, false);
    if (ImGui::BeginMenu("Create Entity")) {
      ImGui::MenuItem("Entity Types", nullptr, nullptr, false);
      if (ImGui::MenuItem("Null Entity")) {
        ECS::EManager.AddNewEntity();
      }
      ImGui::Separator();
      if (ImGui::MenuItem("Cube")) {
        auto cube = ECS::EManager.AddNewEntity();
        cube->AddComponent<BaseMaterial>();
        cube->AddComponent<MeshRenderer>(
            Core.RManager.GetPrimitive(Resource::PRIMITIVE_TYPE::CUBE));
      }
      if (ImGui::MenuItem("Plane")) {
        auto plane = ECS::EManager.AddNewEntity();
        plane->AddComponent<BaseMaterial>();
        plane->AddComponent<MeshRenderer>(
            Core.RManager.GetPrimitive(Resource::PRIMITIVE_TYPE::PLANE));
      }
      if (ImGui::MenuItem("Sphere")) {
        auto sphere = ECS::EManager.AddNewEntity();
        sphere->AddComponent<BaseMaterial>();
        sphere->AddComponent<MeshRenderer>(
            Core.RManager.GetPrimitive(Resource::PRIMITIVE_TYPE::SPHERE));
      }
      if (ImGui::MenuItem("Cylinder")) {
        auto cylinder = ECS::EManager.AddNewEntity();
        cylinder->AddComponent<BaseMaterial>();
        cylinder->AddComponent<MeshRenderer>(
            Core.RManager.GetPrimitive(Resource::PRIMITIVE_TYPE::CYLINDER));
      }
      if (ImGui::MenuItem("Cone")) {
        auto cone = ECS::EManager.AddNewEntity();
        cone->AddComponent<BaseMaterial>();
        cone->AddComponent<MeshRenderer>(
            Core.RManager.GetPrimitive(Resource::PRIMITIVE_TYPE::CONE));
      }
      ImGui::EndMenu();
    }
    ImGui::EndPopup();
  }
  ImGui::BeginChild("Entities List", {-1, ImGui::GetContentRegionAvail().y});
  auto entities = ECS::EManager.HierarchyRoots;
  static ImGuiTreeNodeFlags guiTreeNodeFlags =
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
      ImGuiTreeNodeFlags_SpanAvailWidth;
  for (auto i = 0; i < entities.size(); ++i) {
    DrawHierarchyGUI(entities[i], selectedEntity, guiTreeNodeFlags);
  }
  ImGui::EndChild();
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("IMPORT_MODEL_ASSETS")) {
      char *modelPath = (char *)payload->Data;
      // Console.Log(modelPath);
      auto modelEntity = Core.RManager.GetModelEntity(modelPath);
    }
    ImGui::EndDragDropTarget();
  }
  ImGui::End();
}

void EditorWindows::ConsoleWindow() { Console.Draw("Console"); }

void DrawFileHierarchy(string parentPath, int &parentTreeNodeInd,
                       ImGuiTreeNodeFlags parentFlag, int &selectedFile) {
  for (const auto &entry : fs::directory_iterator(parentPath)) {
    ImGuiTreeNodeFlags finalFlags = parentFlag;
    bool isDirectory = fs::is_directory(entry);
    if (!isDirectory)
      finalFlags |= ImGuiTreeNodeFlags_Bullet;
    if (selectedFile == parentTreeNodeInd)
      finalFlags |= ImGuiTreeNodeFlags_Selected;
    bool isOpen = ImGui::TreeNodeEx((entry.path().filename().string() + "##" +
                                     std::to_string(parentTreeNodeInd++))
                                        .c_str(),
                                    finalFlags);
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
      selectedFile = parentTreeNodeInd - 1;
    // judge the file type from its extension
    string fileExtension = entry.path().extension().string();
    if (!isDirectory) {
      if (fileExtension == ".material") {
        if (ImGui::BeginDragDropSource()) {
          char matFilenameBuffer[100] = {0};
          std::strcpy(matFilenameBuffer, entry.path().string().c_str());
          ImGui::SetDragDropPayload("MATERIAL_OVERRIDE", matFilenameBuffer,
                                    sizeof(matFilenameBuffer));
          ImGui::Text("Drop at a material component to override");
          ImGui::EndDragDropSource();
        }
      } else if (fileExtension == ".obj" || fileExtension == ".fbx" ||
                 fileExtension == ".gltf") {
        if (ImGui::BeginDragDropSource()) {
          char modelFilenameBuffer[100] = {0};
          std::strcpy(modelFilenameBuffer, entry.path().string().c_str());
          ImGui::SetDragDropPayload("IMPORT_MODEL_ASSETS", modelFilenameBuffer,
                                    sizeof(modelFilenameBuffer));
          ImGui::Text("Drop at the entity window to import");
          ImGui::EndDragDropSource();
        }
      } else if (fileExtension == ".png" || fileExtension == ".bmp" ||
                 fileExtension == ".jpg") {
        if (ImGui::BeginDragDropSource()) {
          char textureFilenameBuffer[100] = {0};
          std::strcpy(textureFilenameBuffer, entry.path().string().c_str());
          ImGui::SetDragDropPayload("LOAD_TEXTURE", textureFilenameBuffer,
                                    sizeof(textureFilenameBuffer));
          ImGui::Text("Drop at the material texture spot to load");
          ImGui::EndDragDropSource();
        }
      } else if (fileExtension == ".vert" || fileExtension == ".frag" ||
                 fileExtension == ".geom") {
        if (ImGui::BeginDragDropSource()) {
          char textureFilenameBuffer[100] = {0};
          std::strcpy(textureFilenameBuffer, entry.path().string().c_str());
          ImGui::SetDragDropPayload("SHADER_OVERRIDE", textureFilenameBuffer,
                                    sizeof(textureFilenameBuffer));
          ImGui::Text("Drop at the material texture spot to load");
          ImGui::EndDragDropSource();
        }
      }
    }
    // right click context menu
    if (ImGui::BeginPopupContextItem((entry.path().string() + " popup").c_str(),
                                     ImGuiPopupFlags_MouseButtonRight)) {
      if (isDirectory) { // directory options
        ImGui::MenuItem("Directory Options", nullptr, nullptr, false);
        if (ImGui::MenuItem("Remove")) {
          // remove directory
          fs::remove_all(entry.path());
        }
        if (ImGui::BeginMenu("Create")) {
          ImGui::MenuItem("Types", nullptr, nullptr, false);
          if (ImGui::BeginMenu("Empty Folder")) {
            static char folderName[50] = {0};
            ImGui::MenuItem("Folder Name", nullptr, nullptr, false);
            ImGui::PushItemWidth(120);
            ImGui::InputText("##CreateEmptyFolderName", folderName,
                             sizeof(folderName));
            ImGui::PopItemWidth();
            if (ImGui::Button("Create", {-1, 30})) {
              // Console.Log("Create folder %s\n", folderName);
              fs::create_directory(entry.path().string() + "/" + folderName);
              std::strcpy(folderName, "");
              ImGui::CloseCurrentPopup();
            }
            ImGui::EndMenu();
          }
          if (ImGui::BeginMenu("Render")) {
            ImGui::MenuItem("Render", nullptr, nullptr, false);
            if (ImGui::BeginMenu("Base Material")) {
              static char matName[50] = {0};
              ImGui::MenuItem("Material Name", nullptr, nullptr, false);
              ImGui::PushItemWidth(120);
              ImGui::InputText("##BaeMaterialFileName", matName,
                               sizeof(matName));
              ImGui::PopItemWidth();
              if (ImGui::Button("Create", {-1, 30})) {
                // Console.Log("Create folder %s\n", folderName);
                std::ofstream output(entry.path().string() + "/" +
                                     string(matName) + ".material");
                output << Core.RManager.GetDefaultMaterialJson();
                output.close();
                std::strcpy(matName, "");
                ImGui::CloseCurrentPopup();
              }
              ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Vert & Frag Shader")) {
              static char shaderName[50] = {0};
              ImGui::MenuItem("Shader Name", nullptr, nullptr, false);
              ImGui::PushItemWidth(120);
              ImGui::InputText("##BaseShaderFilename", shaderName,
                               sizeof(shaderName));
              ImGui::PopItemWidth();
              if (ImGui::Button("Create", {-1, 30})) {
                // Console.Log("Create folder %s\n", folderName);
                std::ofstream outputVert(entry.path().string() + "/" +
                                         string(shaderName) + ".vert");
                std::ofstream outputFrag(entry.path().string() + "/" +
                                         string(shaderName) + ".frag");
                outputVert
                    << "#version 460 core\n"
                       "layout (location = 0) in vec3 aPos;\n"
                       "layout (location = 1) in vec3 aNormal;\n"
                       "layout (location = 2) in vec2 aTexCoord;\n"
                       "uniform mat4 model;\n"
                       "uniform mat4 view;\n"
                       "uniform mat4 projection;\n"
                       "void main() {\n"
                       "  gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
                       "}\n";
                outputFrag << "#version 460 core\n"
                              "out vec4 FragColor;\n"
                              "void main(){\n"
                              "  FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);\n"
                              "}\n";
                outputVert.close();
                outputFrag.close();
                std::strcpy(shaderName, "");
                ImGui::CloseCurrentPopup();
              }
              ImGui::EndMenu();
            }
            if (ImGui::MenuItem("Geometry Shader")) {
            }
            ImGui::EndMenu();
          }
          ImGui::EndMenu();
        }
      } else { // file options
        ImGui::MenuItem("File Options", nullptr, nullptr, false);
        if (ImGui::MenuItem("Remove")) {
          fs::remove_all(entry.path());
          // Console.Log("Remove file : %s\n", entry.path().string().c_str());
        }
      }
      ImGui::EndPopup();
    }
    if (isOpen) {
      if (isDirectory) {
        // draw a diretory
        DrawFileHierarchy(entry.path().string(), parentTreeNodeInd, parentFlag,
                          selectedFile);
      }
      ImGui::TreePop();
    }
  }
}

void EditorWindows::AssetsWindow() {
  ImGui::Begin("Assets");
  static int treeNodeInd = 0, selectedFile = 0;
  treeNodeInd = 0;
  const string rootDir = Core.RManager.GetProjectRootDir();
  if (!fs::exists(rootDir) || !fs::is_directory(rootDir)) {
    cout << "project root dir don't exists or isn't a directory (From "
            "AssetsWindow)"
         << endl;
    return;
  }
  ImGui::SeparatorText("File Hierarchy");
  // Right-click context menu for the parent window
  if (!ImGui::IsAnyItemHovered() &&
      ImGui::IsWindowHovered(ImGuiHoveredFlags_RootAndChildWindows)) {
    // show the right click file context menu
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
      ImGui::OpenPopup("AssetsWindowContextMenu");
    // unselect file
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
      selectedFile = -1;
  }
  // // the right click file context menu
  // if (ImGui::BeginPopup("AssetsWindowContextMenu")) {
  //   ImGui::SeparatorText("File Options");
  //   ImGui::EndPopup();
  // }
  ImGui::BeginChild("File Hierarchy List",
                    {-1, ImGui::GetContentRegionAvail().y});
  ImGuiTreeNodeFlags parentFlag = ImGuiTreeNodeFlags_OpenOnArrow |
                                  ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                  ImGuiTreeNodeFlags_SpanAvailWidth;
  DrawFileHierarchy(rootDir, treeNodeInd, parentFlag, selectedFile);
  ImGui::EndChild();
  ImGui::End();
}

void EditorWindows::DrawGizmos(float x, float y, float width, float height,
                               bool enable) {

  if (ImGui::IsKeyPressed(ImGuiKey_G))
    mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
  if (ImGui::IsKeyPressed(ImGuiKey_R))
    mCurrentGizmoOperation = ImGuizmo::ROTATE;
  if (ImGui::IsKeyPressed(ImGuiKey_S))
    mCurrentGizmoOperation = ImGuizmo::SCALE;

  ECS::EntityID camera;
  if (enable && GetActiveCamera(camera)) {
    ImGuizmo::AllowAxisFlip(false);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(x, y, width, height);
    Entity *cameraEnt = ECS::EManager.EntityFromID(camera);
    Camera &cameraComp = cameraEnt->GetComponent<Camera>();
    if (showGizmoGrid) {
      ImGuizmo::DrawGrid(
          glm::value_ptr(cameraComp.GetViewMatrix(*cameraEnt)),
          glm::value_ptr(cameraComp.GetProjMatrixPerspective(width, height)),
          glm::value_ptr(mat4(1.0f)), gizmoGridSize);
    }
    if (selectedEntity != (ECS::EntityID)(-1)) {
      Entity *selected = ECS::EManager.EntityFromID(selectedEntity);
      mat4 modelTransform = selected->GetModelMatrix();
      ImGuizmo::Manipulate(
          glm::value_ptr(cameraComp.GetViewMatrix(*cameraEnt)),
          glm::value_ptr(cameraComp.GetProjMatrixPerspective(width, height)),
          mCurrentGizmoOperation, mCurrentGizmoMode,
          glm::value_ptr(modelTransform), NULL, NULL);
      if (ImGuizmo::IsUsing()) {
        // update object transform with modified changes
        if (mCurrentGizmoOperation == ImGuizmo::TRANSLATE) {
          vec3 position(modelTransform[3][0], modelTransform[3][1],
                        modelTransform[3][2]);
          selected->SetGlobalPosition(position);
        } else {
          vec3 scale(glm::length(modelTransform[0]),
                     glm::length(modelTransform[1]),
                     glm::length(modelTransform[2]));

          if (mCurrentGizmoOperation == ImGuizmo::ROTATE) {
            mat4 rotation =
                mat4(modelTransform[0] / scale.x, modelTransform[1] / scale.y,
                     modelTransform[2] / scale.z, vec4(0.0f, 0.0f, 0.0f, 1.0f));
            selected->SetGlobalRotation(glm::quat_cast(rotation));
          } else if (mCurrentGizmoOperation == ImGuizmo::SCALE)
            selected->SetGlobalScale(scale);
        }
      }
    }
  }
}

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
            ("Shaders: " + material.matData->shader->identifier).c_str(), nullptr, nullptr, false);
      if (ImGui::Button("Reset")) {
        material.matData->SetShader(); // reset to defualt shader
      }
      ImGui::SameLine();
      if (ImGui::Button("Reload")) {
        material.matData->shader = Core.RManager.GetShader(
            material.matData->shader->vertexShaderPath, material.matData->shader->fragShaderPath, "none", true);
      }
      ImGui::TextWrapped("Vertex Shader: %s", material.matData->shader->vertexShaderPath.c_str());
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
      ImGui::Combo("Type", &selectedNewVariableType,
                   newVariableTypes, 6);
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
            material.matData->AddVariable(
                newVariableName,
                Core.RManager.GetIcon(Resource::ICON_TYPE::NULL_TYPE));
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
      if (texVar.second->type == Resource::TEXTURE_TYPE::ICON_TEXTURE) {
        // can be replaced with new texture
        ImGui::Image(
            (void *)Core.RManager.GetIcon(Resource::ICON_TYPE::NULL_TYPE)->id,
            {textureSize, textureSize});
        if (ImGui::BeginDragDropTarget()) {
          if (const ImGuiPayload *payload =
                  ImGui::AcceptDragDropPayload("LOAD_TEXTURE")) {
            char *texturePath = (char *)payload->Data;
            auto newTexture = Core.RManager.GetTexture(texturePath);
            texVar.second =
                newTexture; // replace upload texture with actual loaded texture
          }
          ImGui::EndDragDropTarget();
        }
      } else {
        ImGui::Image((void *)texVar.second->id, {textureSize, textureSize});
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
  if (ImGui::Button("Add Component", {-1, 40})) {
    Console.Log("add component\n");
  }
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

void EditorWindows::RenderStart(Graphics::FrameBuffer *sceneBuffer) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  MainMenuBar();

  ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
  ImGui::Begin("Scene");
  ImGui::BeginChild("GameRenderer");
  auto size = ImGui::GetContentRegionAvail();
  auto pos = ImGui::GetWindowPos();
  ImGui::Image((void *)sceneBuffer->GetFrameTexture(), size, ImVec2(0, 1),
               ImVec2(1, 0));
  SceneWindowSize = {size.x, size.y};
  SceneWindowPos = {pos.x, pos.y};
  DrawGizmos(pos.x, pos.y, size.x, size.y);
  ImGui::EndChild();
  ImGui::End();

  ImGui::ShowDemoWindow();

  EntitiesWindow();
  ConsoleWindow();
  InspectorWindow();
  AssetsWindow();
}

void EditorWindows::RenderComplete() {
  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool EditorWindows::LoopCursorInSceneWindow() {
  vec2 cursorPos = Event.MouseCurrentPosition;
  if (!InSceneWindow(cursorPos.x, cursorPos.y)) {
    cursorPos -= SceneWindowPos;
    while (cursorPos.x < 0.0f)
      cursorPos.x += SceneWindowSize.x;
    while (cursorPos.x > SceneWindowSize.x)
      cursorPos.x -= SceneWindowSize.x;
    while (cursorPos.y < 0.0f)
      cursorPos.y += SceneWindowSize.y;
    while (cursorPos.y > SceneWindowSize.y)
      cursorPos.y -= SceneWindowSize.y;
    cursorPos += SceneWindowPos;
    glfwSetCursorPos(&Core.Window(), cursorPos.x, cursorPos.y);
    return false;
  } else
    return true;
}

bool EditorWindows::SetActiveCamera(ECS::EntityID camera) {
  if (ECS::EManager.HasComponent<Camera>(camera)) {
    hasActiveCamera = true;
    activeCamera = camera;
    return true;
  } else {
    Console.Log("[error]: Not a valid camera entity: %ld\n", camera);
    // there could exist an active camera,
    // don't reset the hasActiveCamera flag
    return false;
  }
}

bool EditorWindows::GetActiveCamera(ECS::EntityID &camera) {
  if (hasActiveCamera) {
    camera = activeCamera;
    return true;
  } else {
    Console.Log("[Info]: There's no active camera\n");
    camera = (ECS::EntityID)(-1);
    return false;
  }
}