/**
 * This file contains some implementation function in Base folder.
 */
#include "Global.hpp"
#include "Base/Scriptable.hpp"
#include "Base/BaseComponent.hpp"

namespace aEngine {

std::string Scriptable::getTypeName() {
  return std::string(typeid(*this).name());
}

void Scriptable::drawInspectorGUIDefault() {
  // Simply display the name of this native script
  ImGui::Separator();
  ImGui::MenuItem("Script Name: ", nullptr, nullptr, false);
  ImGui::TextWrapped(getTypeName().c_str());
  ImGui::SameLine(ImGui::GetContentRegionMax().x - 40);
  ImGui::Checkbox("##enable", &Enabled);
  ImGui::Separator();
}

void Scriptable::DrawInspectorGUI() {
  drawInspectorGUIDefault();
}

};