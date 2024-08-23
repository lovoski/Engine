#pragma once

#include "Entity.hpp"
#include "Base/BaseComponent.hpp"

#include "Function/Render/Mesh.hpp"
#include "Function/Animation/Motion.hpp"

namespace aEngine {

class BaseDeformer;

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

  // All the meshes to be deformed,
  // when one mesh is to be updated by the animator,
  // the entity holding the MeshRenderer should keep it from 
  // updated by the hierarchy system.
  std::vector<Render::Mesh *> meshes;
};

}; // namespace aEngine