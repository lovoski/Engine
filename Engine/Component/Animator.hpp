#pragma once

#include "Base/BaseComponent.hpp"
#include "Entity.hpp"

#include "Function/Animation/Motion.hpp"
#include "Function/Render/Buffers.hpp"
#include "Function/Render/Mesh.hpp"

namespace aEngine {

class BaseDeformer;

// Each animator must bind to one actor (Skeleton)
struct Animator : public BaseComponent {
  Animator(Animation::Skeleton *act) : actor(act) {
    jointActive.resize(actor->GetNumJoints(), 1);
  }
  Animator(Animation::Motion *m) : motion(m) {
    actor = &motion->skeleton;
    jointActive.resize(actor->GetNumJoints(), 1);
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
  std::vector<int> jointActive;
  Animation::Skeleton *actor;

  // Stores the motion data
  Animation::Motion *motion = nullptr;

private:
  void DrawSkeletonHierarchy();
};

}; // namespace aEngine