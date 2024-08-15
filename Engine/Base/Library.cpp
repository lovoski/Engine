/**
 * This file contains some implementation function in Base folder.
 */
#include "Global.hpp"
#include "Base/Scriptable.hpp"

namespace aEngine {

std::string Scriptable::GetTypeName() {
  return std::string(typeid(*this).name());
}

void Scriptable::DrawInspectorGUIDefault() {
  // Simply display the name of this native script
  ImGui::Separator();
  ImGui::MenuItem("Script Name: ", nullptr, nullptr, false);
  ImGui::TextWrapped(GetTypeName().c_str());
  ImGui::Separator();
}

void Scriptable::DrawInspectorGUI() {
  DrawInspectorGUIDefault();
}

};