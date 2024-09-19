#pragma once

#include "Global.hpp"

#include "Function/AssetsType.hpp"

namespace aEngine {

namespace GUIUtils {

void ColorEdit3(std::string label, glm::vec3 &color);
void ColorEdit4(std::string label, glm::vec4 &color);
void DragableTextureTarget(std::string label, Texture &texture,
                           bool flipVertically = true);
// `augInd` is the index to an augmented names with `None:-1` as the
// first element, use the parameter of `handleCurrent` to properly
// access the original `names` array. If the parameter for `handleCurrent`
// is -1, means `None:-1` is selected
void Combo(
    std::string label, std::vector<std::string> &names, int &augInd,
    std::function<void(int)> handleCurrent = [](int) {});
void DragdropTarget(std::string source,
                    std::function<void(void *)> handlePayload);
// Load file or directory from ASSET_FILENAME drag drop source,
// `label` should be unique for each target.
// The function `handleLoad` returns true if
// the file is considered successfully loaded.
void DragableFileTarget(
    std::string label, std::string hint,
    std::function<bool(std::string)> handleLoad,
    std::vector<char> &filenameBuffer,
    std::function<void(void)> handleClear = []() {});

}; // namespace GUIUtils

}; // namespace aEngine