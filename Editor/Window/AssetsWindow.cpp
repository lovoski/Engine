#include "../Editor.hpp"

#include "Component/Camera.hpp"
#include "Component/MeshRenderer.hpp"
#include "Component/Light.hpp"

#include "Function/Render/Shader.hpp"
#include "Function/Render/RenderPass.hpp"

using glm::vec2;
using glm::vec3;
using glm::vec4;
using std::string;
using std::vector;

void DirectoryRightClickMenu(fs::directory_entry entry) {
  ImGui::MenuItem("Directory Options", nullptr, nullptr, false);
  if (ImGui::MenuItem("Remove")) {
    // remove directory
    fs::remove_all(entry.path());
  }
  if (ImGui::BeginMenu("Create")) {
    ImGui::MenuItem("Types", nullptr, nullptr, false);
    if (ImGui::BeginMenu("Empty Scene")) {
      static char sceneName[50] = {0};
      ImGui::MenuItem("Scene Name", nullptr, nullptr, false);
      ImGui::PushItemWidth(120);
      ImGui::InputText("##createemptyscenename", sceneName,
                       sizeof(sceneName));
      ImGui::PopItemWidth();
      if (ImGui::Button("Create", {-1, 30})) {
        string path = entry.path().string() + "/" + sceneName + ".scene";
        std::ofstream sceneFileOutput(path);
        if (!sceneFileOutput.is_open()) {
          LOG_F(ERROR, "failed to create scene file at %s", path.c_str());
        } else {
          // Json json;
          // // set up a null scene
          // json["scene"]["activeCamera"] = -1;
          // sceneFileOutput << json;
          LOG_F(ERROR, "create scene file at %s", path.c_str());
        }
        sceneFileOutput.close();
        std::strcpy(sceneName, "");
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Empty Folder")) {
      static char folderName[50] = {0};
      ImGui::MenuItem("Folder Name", nullptr, nullptr, false);
      ImGui::PushItemWidth(120);
      ImGui::InputText("##CreateEmptyFolderName", folderName,
                       sizeof(folderName));
      ImGui::PopItemWidth();
      if (ImGui::Button("Create", {-1, 30})) {
        fs::create_directory(entry.path().string() + "/" + folderName);
        std::strcpy(folderName, "");
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndMenu();
    }
    ImGui::EndMenu();
  }
}

void FileRightClickMenu(fs::directory_entry entry) {
  ImGui::MenuItem("File Options", nullptr, nullptr, false);
  if (ImGui::MenuItem("Remove")) {
    fs::remove_all(entry.path());
  }
}

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
    {
      // make the file or directory dragable
      if (ImGui::BeginDragDropSource()) {
        char nameBuffer[200] = {0};
        std::strcpy(nameBuffer, entry.path().string().c_str());
        ImGui::SetDragDropPayload("ASSET_FILENAME", nameBuffer,
                                  sizeof(nameBuffer));
        ImGui::Text("Drop at the entity window to import");
        ImGui::EndDragDropSource();
      }
    }
    // right click context menu
    if (ImGui::BeginPopupContextItem((entry.path().string() + " popup").c_str(),
                                     ImGuiPopupFlags_MouseButtonRight)) {
      if (isDirectory) { // directory options
        DirectoryRightClickMenu(entry);
      } else { // file options
        FileRightClickMenu(entry);
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

void Editor::AssetsWindow() {
  ImGui::Begin("Assets", &showAssetsWindow);
  static int treeNodeInd = 0, selectedFile = 0;
  treeNodeInd = 0;
  const string rootDir = context.activeBaseFolder;
  if (!fs::exists(rootDir) || !fs::is_directory(rootDir)) {
    LOG_F(ERROR, "project root dir don't exists or isn't a directory");
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
  // the right click file context menu
  if (ImGui::BeginPopup("AssetsWindowContextMenu")) {
    // DirectoryRightClickMenu(fs::directory_entry(context.activeBaseFolder));
    ImGui::EndPopup();
  }
  ImGui::BeginChild("File Hierarchy List",
                    {-1, ImGui::GetContentRegionAvail().y});
  ImGuiTreeNodeFlags parentFlag = ImGuiTreeNodeFlags_OpenOnArrow |
                                  ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                  ImGuiTreeNodeFlags_SpanAvailWidth;
  DrawFileHierarchy(rootDir, treeNodeInd, parentFlag, selectedFile);
  ImGui::EndChild();
  ImGui::End();
}
