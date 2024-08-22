#pragma once

#include "Base/BaseComponent.hpp"
#include "Entity.hpp"
#include "Function/Animation/Motion.hpp"

namespace aEngine {

struct Animator : public BaseComponent {
  Animator() {}
  Animator(Animation::Motion *m) : motion(m) {
    // the default pose
    CurrentPose = motion->GetRestPose();
  }
  ~Animator() {}

  void DrawInspectorGUI() override {
    if (ImGui::TreeNode("Animator")) {
      ImGui::MenuItem("Skeleton", nullptr, nullptr, false);
      ImGui::Checkbox("Show Skeleton", &ShowSkeleton);
      ImGui::Checkbox("Skeleton On Top", &SkeletonOnTop);
      // static char rootEntityName[100] = {0};
      // ImGui::InputTextWithHint("##skeletonentity", "Skeleton Root Entity",
      //                          rootEntityName, sizeof(rootEntityName),
      //                          ImGuiInputTextFlags_ReadOnly);
      // if (ImGui::BeginDragDropTarget()) {
      //   if (const ImGuiPayload *payload =
      //           ImGui::AcceptDragDropPayload("ENTITYID_DATA")) {
      //     Entity *skeletonRoot = *(Entity **)payload->Data;
      //     skeleton = skeletonRoot;
      //   }
      //   ImGui::EndDragDropTarget();
      // }
      float skeletonColor[3] = {SkeletonColor.x, SkeletonColor.y,
                                SkeletonColor.z};
      if (ImGui::ColorEdit3("Skeleton Color", skeletonColor)) {
        SkeletonColor =
            glm::vec3(skeletonColor[0], skeletonColor[1], skeletonColor[2]);
      }
      ImGui::MenuItem("Motion", nullptr, nullptr, false);
      ImGui::TextWrapped("FPS: %d", motion == nullptr ? -1 : motion->fps);
      ImGui::TreePop();
    }
  }

  // Get transformation matrics needed for skeleton animation
  std::vector<glm::mat4> GetSkeletonTransforms();

  // Skeleton visualization related
  // This entity should be the root joint
  Entity *skeleton = nullptr;
  bool ShowSkeleton = true;
  bool SkeletonOnTop = false;
  // Color for the visualized skeleton
  glm::vec3 SkeletonColor = glm::vec3(0.0f, 1.0f, 0.0f);

  // Cache pose data at this frame, this property
  // is updated by the AnimationSystem each frame
  Animation::Pose CurrentPose;

  // Stores the motion data
  Animation::Motion *motion = nullptr;
};

}; // namespace aEngine