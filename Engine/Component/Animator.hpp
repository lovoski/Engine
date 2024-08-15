#pragma once

#include "Entity.hpp"
#include "Base/BaseComponent.hpp"
#include "Utils/Animation/Motion.hpp"

namespace aEngine {

struct Animator : public BaseComponent {
  Animator() {}
  Animator(Animation::Motion *m) : motion(m) {
    // the default pose
    CurrentPose = motion->poses[0];
  }
  ~Animator() {}

  // Skeleton visualization related
  // This entity should be the root joint
  Entity *skeleton = nullptr;
  bool ShowSkeleton = true;
  // Color for the visualized skeleton
  glm::vec3 SkeletonColor = glm::vec3(0.0f, 1.0f, 0.0f);

  // Cache pose data at this frame
  Animation::Pose CurrentPose;

  // Stores the motion data
  Animation::Motion *motion = nullptr;
};

};