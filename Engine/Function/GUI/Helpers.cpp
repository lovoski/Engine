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
      } else {
        LOG_F(ERROR, "only .png, .tga and .jpg texture are supported");
      }
    }
    ImGui::EndDragDropTarget();
  }
  ImGui::SameLine();
  ImGui::TextWrapped(label.c_str());
}

void Combo(std::string label, std::vector<std::string> &names, int &current,
           std::function<void(int)> handleCurrent) {
  if (current >= names.size())
    current = 0; // set current to null when index out of range
  if (ImGui::BeginCombo(label.c_str(), names[current].c_str())) {
    for (int comboIndex = 0; comboIndex < names.size(); ++comboIndex) {
      bool isSelected = current == comboIndex;
      if (ImGui::Selectable(names[comboIndex].c_str(), isSelected)) {
        current = comboIndex;
        handleCurrent(current);
      }
      if (isSelected)
        ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
}

}; // namespace GUIUtils

}; // namespace aEngine