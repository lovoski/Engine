/**
 * This file contains some implementation function in Base folder.
 */
#include "Base/BaseComponent.hpp"
#include "Base/Scriptable.hpp"
#include "Global.hpp"

namespace aEngine {

std::size_t BaseComponent::HashString(std::string str) {
  const int p1 = 31;
  const int p2 = 53;
  const int m = 1e9 + 9;

  size_t hash1 = 0;
  size_t hash2 = 0;
  size_t p_pow1 = 1;
  size_t p_pow2 = 1;

  for (char c : str) {
    hash1 = (hash1 + (c - 'a' + 1) * p_pow1) % m;
    hash2 = (hash2 + (c - 'a' + 1) * p_pow2) % m;
    p_pow1 = (p_pow1 * p1) % m;
    p_pow2 = (p_pow2 * p2) % m;
  }

  return hash1 ^ (hash2 << 1); // Combining two hash values
}

std::string Scriptable::getInspectorWindowName() {
  return std::string(typeid(*this).name());
}

void Scriptable::OnEnable() {
  LOG_F(INFO, "call OnEnable for %s", getInspectorWindowName().c_str());
}

void Scriptable::OnDisable() {
  LOG_F(INFO, "call OnDisable for %s", getInspectorWindowName().c_str());
}

void Scriptable::DrawInspectorGUIInternal() {
  if (ImGui::Checkbox("Enable Script", &Enabled)) {
    if (Enabled)
      OnEnable();
    else
      OnDisable();
  }
  ImGui::Separator();
  if (!Enabled)
    ImGui::BeginDisabled();
  DrawInspectorGUI();
  if (!Enabled)
    ImGui::EndDisabled();
}

}; // namespace aEngine