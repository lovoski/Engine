/**
 * This file contains some implementation function in Base folder.
 */
#include "Base/BaseComponent.hpp"
#include "Base/Scriptable.hpp"
#include "Global.hpp"

namespace aEngine {

std::string Scriptable::getTypeName() {
  return std::string(typeid(*this).name());
}

void Scriptable::OnEnable() {
  LOG_F(INFO, "call OnEnable for %s", getTypeName().c_str());
}

void Scriptable::OnDisable() {
  LOG_F(INFO, "call OnDisable for %s", getTypeName().c_str());
}

void Scriptable::DrawInspectorGUI() {
  if (ImGui::Checkbox("Enable Script", &Enabled)) {
    if (Enabled)
      OnEnable();
    else
      OnDisable();
  }
  ImGui::Separator();
  if (!Enabled)
    ImGui::BeginDisabled();
  drawCustomInspectorGUI();
  if (!Enabled)
    ImGui::EndDisabled();
}

}; // namespace aEngine