#include "Component/NativeScript.hpp"

#include "Scripts/Animation/MotionMatching.hpp"
#include "Scripts/Animation/SAMERetarget.hpp"
#include "Scripts/Animation/VisMetrics.hpp"
#include "Scripts/Geometry/SelfIntersection.hpp"
#include "Scripts/Geometry/PolygonMeshProcessing.hpp"
#include "Scripts/Animation/IK/TwoBoneIK.hpp"

namespace aEngine {

NativeScript::~NativeScript() {
  // free all scriptable instance
  instances.clear();
}

void NativeScript::drawAddScriptPopup() {
  if (ImGui::BeginPopup("addscriptpanelpopup")) {
    ImGui::MenuItem("Registered Scripts", nullptr, nullptr, false);

    // animation scripts
    ImGui::Separator();
    if (ImGui::MenuItem("SAME Retarget"))
      Bind<SAMERetarget>();
    if (ImGui::MenuItem("Visual Metrics"))
      Bind<VisMetrics>();
    if (ImGui::MenuItem("Motion Matching"))
      Bind<MotionMatching>();
    if (ImGui::MenuItem("Two Bone IK"))
      Bind<TwoBoneIK>();

    // geometry scripts
    ImGui::Separator();
    if (ImGui::MenuItem("Self Intersection"))
      Bind<SelfIntersection>();
    if (ImGui::MenuItem("Polygon Mesh Processing"))
      Bind<PolygonMeshProcessing>();
    ImGui::EndPopup();
  }
}

void NativeScript::DrawInspectorGUI() {
  if (ImGui::Button("Add Script", {-1, 30}))
    ImGui::OpenPopup("addscriptpanelpopup");
  drawAddScriptPopup();
  for (auto &instance : instances) {
    if (instance != nullptr) {
      // if this is a valid scriptable
      bool nodeOpen =
          ImGui::TreeNodeEx(instance->getInspectorWindowName().c_str());
      if (nodeOpen) {
        instance->DrawInspectorGUIInternal();
        ImGui::TreePop();
      }
    }
  }
}

}; // namespace aEngine

REGISTER_COMPONENT(aEngine, NativeScript);