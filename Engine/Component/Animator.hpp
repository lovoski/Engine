#pragma once

#include "Entity.hpp"
#include "Base/BaseComponent.hpp"

#include "Function/Render/Mesh.hpp"
#include "Function/Render/Buffers.hpp"
#include "Function/Animation/Motion.hpp"

namespace aEngine {

class BaseDeformer;

// Each animator must bind to one actor (Skeleton)
struct Animator : public BaseComponent {
  Animator(Animation::Skeleton *act) : actor(act) {}
  Animator(Animation::Motion *m) : motion(m) {
    actor = &motion->skeleton;
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

  // Each animator must specify one actor
  Animation::Skeleton *actor;

  // Stores the motion data
  Animation::Motion *motion = nullptr;
};

}; // namespace aEngine