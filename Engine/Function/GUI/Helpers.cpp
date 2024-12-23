#include "Function/GUI/Helpers.hpp"
#include "Function/AssetsLoader.hpp"

namespace aEngine {

namespace GUIUtils {

void ColorEdit3(std::string label, glm::vec3 &color) {
  float c[3] = {color.x, color.y, color.z};
  if (ImGui::ColorEdit3(label.c_str(), c)) {
    color.x = c[0];
    color.y = c[1];
    color.z = c[2];
  }
}
void ColorEdit4(std::string label, glm::vec4 &color) {
  float c[4] = {color.x, color.y, color.z, color.w};
  if (ImGui::ColorEdit4(label.c_str(), c)) {
    color.x = c[0];
    color.y = c[1];
    color.z = c[2];
    color.w = c[3];
  }
}

void DragableTextureTarget(std::string label, Texture &texture,
                           bool flipVertically) {
  if (ImGui::Button(("Reset##" + label).c_str())) {
    static Texture *null = Loader.GetTexture("::null_texture");
    texture.id = null->id;
    texture.path = null->path;
  }
  ImGui::SameLine();
  ImGui::Image((void *)(static_cast<std::uintptr_t>(texture.id)), {128, 128});
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("ASSET_FILENAME")) {
      char *assetFilename = (char *)payload->Data;
      fs::path filepath = fs::path(assetFilename);
      std::string extension = filepath.extension().string();
      if (extension == ".png" || extension == ".tga" || extension == ".jpg") {
        auto loadedTexture =
            Loader.GetTexture(filepath.string(), flipVertically);
        texture.id = loadedTexture->id;
        texture.path = loadedTexture->path;
      } else if (extension == ".hdr") {
        auto loadedTexture =
            Loader.GetHDRTexture(filepath.string(), flipVertically);
        texture.id = loadedTexture->id;
        texture.path = loadedTexture->path;
      } else {
        LOG_F(ERROR, "only .png, .tga, .jpg and .hdr texture are supported");
      }
    }
    ImGui::EndDragDropTarget();
  }
  ImGui::SameLine();
  ImGui::TextWrapped(label.c_str());
}

void Combo(std::string label, std::vector<std::string> &names, int &augInd,
           std::function<void(int)> handleCurrent) {
  std::vector<std::string> augNames{"None:-1"};
  augNames.insert(augNames.end(), names.begin(), names.end());
  if (augInd >= augNames.size())
    augInd = 0; // set current to null when index out of range
  if (ImGui::BeginCombo(label.c_str(), augNames[augInd].c_str())) {
    for (int comboIndex = 0; comboIndex < augNames.size(); ++comboIndex) {
      bool isSelected = augInd == comboIndex;
      if (ImGui::Selectable(augNames[comboIndex].c_str(), isSelected)) {
        augInd = comboIndex;
        handleCurrent(augInd - 1);
      }
      if (isSelected)
        ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
}

void DragdropTarget(std::string source,
                    std::function<void(void *)> handlePayload) {
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload(source.c_str())) {
      handlePayload(payload->Data);
    }
    ImGui::EndDragDropTarget();
  }
}

void DragableFileTarget(std::string label, std::string hint,
                        std::function<bool(std::string)> handleLoad,
                        std::vector<char> &filenameBuffer,
                        std::function<void(void)> handleClear) {
  ImGui::BeginChild((label + "##children" + label).c_str(), {-1, 30});
  ImGui::InputTextWithHint(("##inputwithhint" + label).c_str(), hint.c_str(),
                           filenameBuffer.data(), filenameBuffer.size(),
                           ImGuiInputTextFlags_ReadOnly);
  ImGui::SameLine();
  if (ImGui::Button(("Clear##clear" + label).c_str(), {-1, -1})) {
    handleClear();
    for (int i = 0; i < filenameBuffer.size(); ++i)
      filenameBuffer[i] = 0;
  }
  ImGui::EndChild();
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("ASSET_FILENAME")) {
      fs::path filepath = fs::path((char *)payload->Data);
      if (handleLoad(filepath.string())) {
        sprintf(filenameBuffer.data(), filepath.string().c_str());
      }
    }
    ImGui::EndDragDropTarget();
  }
}

void DragableEntityTarget(std::string label, std::string hint,
                          std::function<bool(Entity *)> handleLoad,
                          std::vector<char> &nameBuffer,
                          std::function<void(void)> handleClear) {
  ImGui::BeginChild((label + "##children" + label).c_str(), {-1, 30});
  ImGui::InputTextWithHint(("##inputwithhint" + label).c_str(), hint.c_str(),
                           nameBuffer.data(), nameBuffer.size(),
                           ImGuiInputTextFlags_ReadOnly);
  ImGui::SameLine();
  if (ImGui::Button(("Clear##clear" + label).c_str(), {-1, -1})) {
    handleClear();
    for (int i = 0; i < nameBuffer.size(); ++i)
      nameBuffer[i] = 0;
  }
  ImGui::EndChild();
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("ENTITYID_DATA")) {
      auto entity = *(Entity **)payload->Data;
      if (handleLoad(entity)) {
        sprintf(nameBuffer.data(), "%s", entity->name.c_str());
      }
    }
    ImGui::EndDragDropTarget();
  }
}

}; // namespace GUIUtils

}; // namespace aEngine