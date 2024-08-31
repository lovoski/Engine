/**
 * Each animator must bind to one actor containing the description info
 * to the original skeleton, the animator will create a hierarchy of entities
 * matching the names of this actor.
 * Mismatching between the actor and the skeleton entities is only allowed when
 * the skeleton entities joints is a subset of this actor (so we can set Rest Pose).
 * Motion will be applied to the skeleton entities when the skeleton entities is a 
 * subset of the motion actor.
 */
#pragma once

#include "Base/BaseComponent.hpp"
#include "Entity.hpp"

#include "Function/Animation/Motion.hpp"
#include "Function/Render/Buffers.hpp"
#include "Function/Render/Mesh.hpp"

namespace aEngine {

// Each animator must bind to one actor (Skeleton),
// the entity structure will be created when one animator gets created
struct Animator : public BaseComponent {
  Animator(Animation::Skeleton *act) : actor(act) {
    jointActive.resize(actor->GetNumJoints(), 1);
    createSkeletonEntities();
  }
  Animator(Animation::Motion *m) : motion(m) {
    actor = &motion->skeleton;
    jointActive.resize(actor->GetNumJoints(), 1);
    createSkeletonEntities();
  }
  ~Animator() {}

  void DrawInspectorGUI() override;

  // Get transformation matrics needed for skeleton animation
  std::vector<glm::mat4> GetSkeletonTransforms();

  // Apply the motion to skeleton entities
  void ApplyMotionToSkeleton(Animation::Pose &pose);
  // Build a map for skeleton entities, the key for this map is the name
  // of the entity. 
  // Parent entity will always appear before child entities in this map.
  // The parameter `onlyActiveJoints` will remove joint entity appears
  // inactive in member variable `jointActive`
  void BuildSkeletonMap(std::map<std::string, Entity *> &skeletonMap,
                        bool onlyActiveJoints = false);

  // Skeleton visualization related
  // This entity should be the root joint
  Entity *skeleton = nullptr;
  bool ShowSkeleton = true;
  bool SkeletonOnTop = false;
  // Color for the visualized skeleton
  glm::vec3 SkeletonColor = glm::vec3(0.0f, 1.0f, 0.0f);

  // name info
  std::string skeletonName = "", motionName = "";

  // 0 for inactive, 1 for active
  std::vector<int> jointActive;
  // The actor is a read only reference, motion is 
  // applied to the skeleton entities created from this actor
  Animation::Skeleton *actor;

  // Stores the motion data
  Animation::Motion *motion = nullptr;

private:
  void drawSkeletonHierarchy();
  void createSkeletonEntities();
};

}; // namespace aEngine