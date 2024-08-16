#pragma once

#include "Base/BaseComponent.hpp"
#include "Entity.hpp"
#include "Utils/Animation/Motion.hpp"

namespace aEngine {

struct Animator : public BaseComponent {
  Animator() {}
  Animator(Animation::Motion *m) : motion(m) {
    // the default pose
    CurrentPose = motion->poses[0];
  }
  ~Animator() {}

  void DrawInspectorGUI() override {
    if (ImGui::TreeNode("Animator")) {
      ImGui::MenuItem("Skeleton", nullptr, nullptr, false);
      ImGui::Checkbox("Show Skeleton", &ShowSkeleton);
      ImGui::Checkbox("Skeleton On Top", &SkeletonOnTop);
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