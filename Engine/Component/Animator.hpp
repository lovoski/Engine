/**
 * Each animator must bind to one actor containing the description info
 * to the original skeleton, the animator will create a hierarchy of entities
 * matching the names of this actor.
 * Mismatching between the actor and the skeleton entities is only allowed when
 * the skeleton entities joints is a subset of this actor (so we can set Rest
 * Pose). Motion will be applied to the skeleton entities when the skeleton
 * entities is a subset of the motion actor.
 */
#pragma once

#include "Base/BaseComponent.hpp"
#include "Entity.hpp"

#include "Function/Animation/Motion.hpp"
#include "Function/Render/Buffers.hpp"
#include "Function/Render/Mesh.hpp"

namespace aEngine {

struct SkeletonMapData {
  bool active;
  int actorInd;
  Entity *joint;
};

// Each animator must bind to one actor (Skeleton),
// the entity structure will be created when one animator gets created
struct Animator : public BaseComponent {
  Animator(EntityID id, Animation::Skeleton *act)
      : actor(act), BaseComponent(id) {
    jointActive.resize(actor->GetNumJoints(), 1);
    createSkeletonEntities();
  }
  Animator(EntityID id, Animation::Motion *m) : motion(m), BaseComponent(id) {
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
  // Maintain member variable `SkeletonMap`, the key for this map is the name
  // of the entity.
  // The function is rather costly, don't call it multiple times in one render
  // loop.
  void BuildSkeletonMap();

  // Skeleton visualization related
  // This entity should be the root joint
  Entity *skeleton = nullptr;
  bool ShowSkeleton = true, ShowJoints = true;
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
  // This map should have the same size as the actor's joints
  std::map<std::string, SkeletonMapData> SkeletonMap;

  // Stores the motion data
  Animation::Motion *motion = nullptr;

private:
  void drawSkeletonHierarchy();
  void createSkeletonEntities();
  // find the index of actor joint matching this name, -1 for not found
  int findActorJointIndWithName(std::string name);
};

}; // namespace aEngine