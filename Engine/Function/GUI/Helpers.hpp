#pragma once

#include "Global.hpp"

#include "Function/AssetsType.hpp"

namespace aEngine {

namespace GUIUtils {

void ColorEdit3(glm::vec3 &color, std::string label);
void ColorEdit4(glm::vec4 &color, std::string label);
void DragableTextureTarget(Texture &texture, std::string name);

}; // namespace GUIUtils

}; // namespace aEngine