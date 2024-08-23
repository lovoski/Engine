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

  void DrawInspectorGUI() override;

  // Get transformation matrics needed for skeleton animation
  std::vector<glm::mat4> GetSkeletonTransforms();

  // Skeleton visualization related
  // This entity should be the root joint
  Entity *skeleton = nullptr;
  bool ShowSkeleton = true;
  bool SkeletonOnTop = false;
  // Color for the visualized skeleton
  glm::vec3 SkeletonColor = glm::vec3(0.0f, 1.0f, 0.0f);

  // name info
  std::string skeletonName = "", motionName = "";

  // Cache pose data at this frame, this property
  // is updated by the AnimationSystem each frame
  Animation::Pose CurrentPose;

  // Stores the motion data
  Animation::Motion *motion = nullptr;
};

}; // namespace aEngine