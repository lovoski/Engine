#pragma once

#include "Base/BaseComponent.hpp"

namespace aEngine {

enum LIGHT_TYPE { DIRECTIONAL_LIGHT, POINT_LIGHT, SPOT_LIGHT };

struct Light : public aEngine::BaseComponent {
  LIGHT_TYPE type = LIGHT_TYPE::DIRECTIONAL_LIGHT;

  glm::vec3 lightColor = glm::vec3(0.5f);

  // directional light specific
  // the directional of light is LocalForward

  // point light specific
  // the position can be found at transform

  // spot light specific

  void DrawInspectorGUI() override {
    if (ImGui::TreeNode("Base Light")) {
      const char *comboItems[] = {"Directional light", "Point light",
                                  "Spot light"};
      static int baseLightGUIComboItemIndex = 0;
      ImGui::Combo("Light Type", &baseLightGUIComboItemIndex, comboItems, 3);
      float color[3] = {lightColor.x, lightColor.y, lightColor.z};
      if (baseLightGUIComboItemIndex == 0) {
        ImGui::ColorEdit3("Light Color", color);
      } else if (baseLightGUIComboItemIndex == 1) {
      } else if (baseLightGUIComboItemIndex == 2) {
      }
      lightColor.x = lightColor[0];
      lightColor.y = lightColor[1];
      lightColor.z = lightColor[2];
      ImGui::TreePop();
    }
  }
};

}; // namespace aEngine
