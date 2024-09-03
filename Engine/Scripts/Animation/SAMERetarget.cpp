#include "Scripts/Animation/SAMERetarget.hpp"

namespace aEngine {

void SAMERetarget::LateUpdate(float dt) {
  auto animator = entity->GetComponent<Animator>();
  if (animator != nullptr && motion != nullptr) {
    // apply motion to animator
    auto animSystem = GWORLD.GetSystemInstance<AnimationSystem>();
    auto currentFrame = animSystem->SystemCurrentFrame;
    auto currentPose = motion->At(currentFrame);
    animator->ApplyMotionToSkeleton(currentPose);
  }
}

void SAMERetarget::DrawInspectorGUI() {
  auto animator = entity->GetComponent<Animator>();
  drawInspectorGUIDefault();
  ImGui::MenuItem("Motion", nullptr, nullptr, false);
  ImGui::BeginChild("choosemotionsource", {-1, 30});
  static char motionSequencePath[200] = {0};
  sprintf(motionSequencePath, motionName.c_str());
  ImGui::InputTextWithHint("##motionsource", "Motion Sequence Path",
                           motionSequencePath, sizeof(motionSequencePath),
                           ImGuiInputTextFlags_ReadOnly);
  ImGui::SameLine();
  if (ImGui::Button("Clear", {-1, -1})) {
    // reset skeleton to rest pose
    if (animator != nullptr)
      animator->ApplyMotionToSkeleton(animator->actor->GetRestPose());
    // clear variables
    motionName = "";
    motion = nullptr;
    for (int i = 0; i < sizeof(motionSequencePath); ++i)
      motionSequencePath[i] = 0;
  }
  ImGui::EndChild();
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("ASSET_FILENAME")) {
      char *assetFilename = (char *)payload->Data;
      fs::path filepath = fs::path(assetFilename);
      std::string extension = filepath.extension().string();
      if (extension == ".bvh" || extension == ".fbx") {
        motion = Loader.GetMotion(filepath.string());
        motionName = filepath.string();
      } else {
        LOG_F(ERROR, "only .bvh and .fbx motion are supported");
      }
    }
    ImGui::EndDragDropTarget();
  }
}

}; // namespace aEngine