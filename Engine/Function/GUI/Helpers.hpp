#pragma once

#include "Global.hpp"

#include "Function/AssetsType.hpp"

namespace aEngine {

namespace GUIUtils {

void ColorEdit3(std::string label, glm::vec3 &color);
void ColorEdit4(std::string label, glm::vec4 &color);
void DragableTextureTarget(std::string label, Texture &texture,
                           bool flipVertically = true);
void Combo(
    std::string label, std::vector<std::string> &names, int &current,
    std::function<void(int)> handleCurrent = [](int) {});
void DragdropTarget(std::string source,
                    std::function<void(void *)> handlePayload);

}; // namespace GUIUtils

}; // namespace aEngine