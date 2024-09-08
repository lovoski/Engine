#include "Component/NativeScript.hpp"

#include "Scripts/Animation/SAMERetarget.hpp"
#include "Scripts/Animation/VisMetrics.hpp"

namespace aEngine {

NativeScript::~NativeScript() {
  // free all scriptable instance
  for (auto ele : instances) {
    ele.second->Destroy();
    delete ele.second;
  }
  instances.clear();
}

void NativeScript::drawAddScriptPopup() {
  if (ImGui::BeginPopup("addscriptpanelpopup")) {
    ImGui::MenuItem("Registered Scripts", nullptr, nullptr, false);
    ImGui::Separator();
    if (ImGui::MenuItem("Visual Metrics"))
      Bind<VisMetrics>();
    if (ImGui::MenuItem("SAME Retarget"))
      Bind<SAMERetarget>();
    ImGui::EndPopup();
  }
}

void NativeScript::DrawInspectorGUI() {
  if (ImGui::Button("Add Script", {-1, 30}))
    ImGui::OpenPopup("addscriptpanelpopup");
  drawAddScriptPopup();
  for (auto instance : instances) {
    if (instance.second != nullptr) {
      // if this is a valid scriptable
      bool nodeOpen = ImGui::TreeNodeEx(instance.second->getTypeName().c_str());
      if (nodeOpen) {
        instance.second->DrawInspectorGUI();
        ImGui::TreePop();
      }
    }
  }
}

}; // namespace aEngine