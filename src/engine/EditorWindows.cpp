#include "EditorWindows.hpp"
#include "resource/MaterialData.hpp"
#include "roboto.h"

// c++ 17 feature
#include <filesystem>

void EditorWindows::Initialize() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

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
      ImGui::SeparatorText("Project");
      if (ImGui::MenuItem("Load Project")) {
      }
      if (ImGui::MenuItem("Save Project")) {
      }
      // ImGui::SeparatorText("Scene");
      // if (ImGui::MenuItem("Save Scene")) {
      //   std::ofstream jsonOut("test_scene.json");
      //   jsonOut << ECS::EManager.CaptureStatesAsScene() << endl;
      //   jsonOut.close();
      // }

      ImGui::EndMenu();
    }
    // if (ImGui::BeginMenu("Tools")) {
    //   // if (ImGui::MenuItem("Show Profiler")) {
    //   //   DrawProfiler();
    //   // }
    //   // if (ImGui::MenuItem("Show Style Editor")) {
    //   //   ImGui::ShowStyleEditor();
    //   // }
    //   ImGui::EndMenu();
    // }
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
  if (ImGui::BeginPopupContextItem((entity->name + " popup").c_str(),
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
  ImGui::BeginChild("Entities List", {-1, ImGui::GetContentRegionAvail().y});
  auto entities = ECS::EManager.HierarchyRoots;
  static ImGuiTreeNodeFlags guiTreeNodeFlags =
      ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
      ImGuiTreeNodeFlags_SpanAvailWidth;
  for (auto i = 0; i < entities.size(); ++i) {
    DrawHierarchyGUI(entities[i], selectedEntity, guiTreeNodeFlags);
  }
  ImGui::EndChild();
  ImGui::End();
}

void EditorWindows::ConsoleWindow() { Console.Draw("Console"); }

void DrawFileHierarchy(string parentPath, int &parentTreeNodeInd,
                       ImGuiTreeNodeFlags parentFlag, int &selectedFile) {
  for (const auto &entry : std::filesystem::directory_iterator(parentPath)) {
    ImGuiTreeNodeFlags finalFlags = parentFlag;
    bool isDirectory = std::filesystem::is_directory(entry);
    if (!isDirectory)
      finalFlags |= ImGuiTreeNodeFlags_Bullet;
    if (selectedFile == parentTreeNodeInd)
      finalFlags |= ImGuiTreeNodeFlags_Selected;
    bool isOpen = ImGui::TreeNodeEx((entry.path().filename().string() + "##" +
                         std::to_string(parentTreeNodeInd++))
                            .c_str(), finalFlags);
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
      selectedFile = parentTreeNodeInd-1;
    // judge the file type from its extension
    string fileExtension = entry.path().extension().string();
    if (!isDirectory && fileExtension == ".material") {
      if (ImGui::BeginDragDropSource()) {
        ImGui::SetDragDropPayload("MATERIAL_OVERRIDE", entry.path().string().c_str(),
                                  entry.path().string().size());
        ImGui::Text("Drop at a material component to override");
        ImGui::EndDragDropSource();
      }
    }
    if (!isDirectory && (fileExtension == ".obj" || fileExtension == ".fbx" || fileExtension == ".gltf")) {
      if (ImGui::BeginDragDropSource()) {
        ImGui::SetDragDropPayload("IMPORT_MODEL_ASSETS", entry.path().string().c_str(),
                                  entry.path().string().size());
        ImGui::Text("Drop at the entity window to import");
        ImGui::EndDragDropSource();
      }
    }
    if (isOpen) {
      if (isDirectory) {
        // draw a diretory
        DrawFileHierarchy(entry.path().string(), parentTreeNodeInd, parentFlag, selectedFile);
      }
      ImGui::TreePop();
    }
  }
}

void EditorWindows::AssetsWindow() {
  ImGui::Begin("Assets");
  const string rootDir = Resource::RManager.GetProjectRootDir();
  if (!std::filesystem::exists(rootDir) ||
      !std::filesystem::is_directory(rootDir)) {
    cout << "project root dir don't exists or isn't a directory (From "
            "AssetsWindow)"
         << endl;
    return;
  }
  ImGui::SeparatorText("File Hierarchy");
  ImGui::BeginChild("File Hierarchy List", {-1, ImGui::GetContentRegionAvail().y});
  static int treeNodeInd = 0, selectedFile = 0;
  treeNodeInd = 0;
  ImGuiTreeNodeFlags parentFlag = ImGuiTreeNodeFlags_OpenOnArrow |
                                  ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                  ImGuiTreeNodeFlags_SpanAvailWidth;
  DrawFileHierarchy(rootDir, treeNodeInd, parentFlag, selectedFile);
  ImGui::EndChild();
  ImGui::End();
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
    if (ImGui::DragFloat3("Rotation", rotations, 1.0f, -180.0f, 180.0f))
      ;
    transform->SetGlobalRotationDegree(
        vec3(rotations[0], rotations[1], rotations[2]));
    if (ImGui::DragFloat3("Scale", scales, 0.01f, -MAX_FLOAT, MAX_FLOAT))
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
    ImGui::SeparatorText(("Material Name: " + material.matIdentifier).c_str());
    // change the texture in use by drag and drop
    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload =
              ImGui::AcceptDragDropPayload("MATERIAL_OVERRIDE")) {
        char *info = (char *)payload->Data;
        Console.Log(info);
        // ECS::EManager.EntityFromID(entity->ID)->AssignChild(newChild);
      }
      ImGui::EndDragDropTarget();
    }
    // shader information
    ImGui::Text("Vertex Shader: %s", material.VertShaderPath.c_str());
    ImGui::Text("Fragment Shader: %s", material.FragShaderPath.c_str());
    // get the pointer
    auto ptr = material.matData;
    // construct dynamic material editor from the maps
    if (ptr->intVariables.size() > 0)
      ImGui::SeparatorText("Int Variables");
    for (auto &intVar : ptr->intVariables) {
      string name = ptr->variableNames[intVar.first];
      if (ImGui::Button(("X##"+name).c_str()))
        ptr->RemoveVariable<int>(name);
      ImGui::SameLine();
      ImGui::InputInt(name.c_str(), &intVar.second);
    }
    if (ptr->floatVariables.size() > 0)
      ImGui::SeparatorText("Float Variables");
    for (auto &floatVar : ptr->floatVariables) {
      float minRange = -10000.0f, maxRange = 10000.0f;
      if (ptr->floatVariablesRange.find(floatVar.first) !=
          ptr->floatVariablesRange.end()) {
        auto range = ptr->floatVariablesRange[floatVar.first];
        minRange = range.first;
        maxRange = range.second;
      }
      string name = ptr->variableNames[floatVar.first];
      if (ImGui::Button(("X##"+name).c_str()))
        ptr->RemoveVariable<float>(name);
      ImGui::SameLine();
      ImGui::SliderFloat(name.c_str(),
                         &floatVar.second, minRange, maxRange);
    }
    if (ptr->vec2Variables.size() > 0)
      ImGui::SeparatorText("Vec2 Variables");
    for (auto &vec2Var : ptr->vec2Variables) {
      string name = ptr->variableNames[vec2Var.first];
      float vec2Value[2] = {vec2Var.second.x, vec2Var.second.y};
      if (ImGui::Button(("X##"+name).c_str()))
        ptr->RemoveVariable<vec2>(name);
      ImGui::SameLine();
      if (ImGui::DragFloat2(name.c_str(), vec2Value))
        ptr->vec2Variables[vec2Var.first] = vec2(vec2Value[0], vec2Value[1]);
    }
    if (ptr->vec3Variables.size() > 0)
      ImGui::SeparatorText("Vec3 Variables");
    for (auto &vec3Var : ptr->vec3Variables) {
      string name = ptr->variableNames[vec3Var.first];
      float vec3Value[3] = {vec3Var.second.x, vec3Var.second.y,
                            vec3Var.second.z};
      if (ImGui::Button(("X##"+name).c_str()))
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
    if (ptr->vec4Variables.size() > 0)
      ImGui::SeparatorText("Vec4 Variables");
    for (auto &vec4Var : ptr->vec4Variables) {
      string name = ptr->variableNames[vec4Var.first];
      float vec4Value[4] = {vec4Var.second.x, vec4Var.second.y,
                            vec4Var.second.z, vec4Var.second.w};
      if (ImGui::Button(("X##"+name).c_str()))
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

void EditorWindows::ComponentsWindow() {
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
  ImGui::EndChild();
  ImGui::End();

  ImGui::ShowDemoWindow();

  EntitiesWindow();
  ConsoleWindow();
  ComponentsWindow();
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