#include "Function/GUI/Helpers.hpp"
#include "Function/AssetsLoader.hpp"

namespace aEngine {

namespace GUIUtils {

void ColorEdit3(glm::vec3 &color, std::string label) {
  float c[3] = {color.x, color.y, color.z};
  if (ImGui::ColorEdit3(label.c_str(), c)) {
    color.x = c[0];
    color.y = c[1];
    color.z = c[2];
  }
}
void ColorEdit4(glm::vec4 &color, std::string label) {
  float c[4] = {color.x, color.y, color.z, color.w};
  if (ImGui::ColorEdit4(label.c_str(), c)) {
    color.x = c[0];
    color.y = c[1];
    color.z = c[2];
    color.w = c[3];
  }
}

void DragableTextureTarget(Texture &texture, std::string name) {
  ImGui::Image((void *)(static_cast<std::uintptr_t>(texture.id)), {128, 128});
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("ASSET_FILENAME")) {
      char *assetFilename = (char *)payload->Data;
      fs::path filepath = fs::path(assetFilename);
      std::string extension = filepath.extension().string();
      if (extension == ".png" || extension == ".tga" || extension == ".jpg") {
        auto loadedTexture = Loader.GetTexture(filepath.string());
        texture.id = loadedTexture->id;
        texture.path = loadedTexture->path;
      } else {
        LOG_F(ERROR, "only .png, .tga and .jpg texture are supported");
      }
    }
    ImGui::EndDragDropTarget();
  }
  ImGui::SameLine();
  ImGui::TextWrapped(name.c_str());
}

}; // namespace GUIUtils

}; // namespace aEngine