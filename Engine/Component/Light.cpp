#include "Component/Light.hpp"

namespace aEngine {

void ModifyLightColor(glm::vec3 &lightColor) {
  float color[3] = {lightColor.x, lightColor.y, lightColor.z};
  if (ImGui::ColorEdit3("Color", color)) {
    lightColor.x = color[0];
    lightColor.y = color[1];
    lightColor.z = color[2];
  }
}

void Light::DrawInspectorGUI() {
  if (ImGui::TreeNode("Base Light")) {
    std::vector<const char *> comboItems = {"Directional light", "Point light"};
    static int baseLightGUIComboItemIndex = 0;
    ImGui::Combo("Light Type", &baseLightGUIComboItemIndex, comboItems.data(),
                 comboItems.size());

    ModifyLightColor(lightColor);

    ImGui::TreePop();
  }
}

}; // namespace aEngine