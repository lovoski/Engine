/**
 * Each animator must bind to one actor containing the description info
 * to the original skeleton, the animator will create a hierarchy of entities
 * matching the names of this actor.
 *
 * Mismatching between the actor and the skeleton entities is only allowed when
 * the actor joints is a subset of skeleton entities.
 */
#pragma once

#include "Base/BaseComponent.hpp"
#include "Entity.hpp"

#include "Function/Animation/Motion.hpp"
#include "Function/Render/Buffers.hpp"
#include "Function/Render/Mesh.hpp"

namespace aEngine {

struct BoneMatrixBlock {
  glm::mat4 BoneModelMatrix;
  glm::mat4 BoneOffsetMatrix;
};

// Each animator must bind to one actor (Skeleton),
// the entity structure will be created when one animator gets created
struct Animator : public BaseComponent {
  Animator() : BaseComponent(0) {}
  Animator(EntityID id, Animation::Skeleton *act);
  Animator(EntityID id, Animation::Motion *m);
  ~Animator();

  void DrawInspectorGUI() override;

  // Get transformation matrics needed for skeleton animation
  std::vector<BoneMatrixBlock> GetSkeletonTransforms();

  // Apply the motion to skeleton entities
  void ApplyPoseToSkeleton(Animation::Pose &pose);
  // Maintain member variables `jointEntityMap`,
  // call this function after you made modifications to the joint entities. (add
  // additional joints, rename joints etc.)
  void BuildMappings();

  std::string getInspectorWindowName() override { return "Animator"; }

  template <typename Archive> void save(Archive &ar) const {
    ar(CEREAL_NVP(entityID));
    std::map<std::size_t, EntityID> jointMapSerialize;
    for (auto &ele : jointEntityMap)
      jointMapSerialize.insert(std::make_pair(ele.first, ele.second->ID));
    ar(skeleton->ID, ShowSkeleton, ShowJoints, JointVisualSize, SkeletonOnTop,
       SkeletonColor, skeletonName, jointActiveMap,
       actor == nullptr ? "none" : actor->path, jointMapSerialize,
       motion == nullptr ? "none" : motion->path, ShowTrajectory, TrajInterval,
       TrajCount);
  }
  template <typename Archive> void load(Archive &ar) {
    ar(CEREAL_NVP(entityID));
    EntityID skelID;
    std::string actorPath, motionPath;
    std::map<std::size_t, EntityID> jointMapSerialize;
    ar(skelID, ShowSkeleton, ShowJoints, JointVisualSize, SkeletonOnTop,
       SkeletonColor, skeletonName, jointActiveMap, actorPath,
       jointMapSerialize, motionPath, ShowTrajectory, TrajInterval, TrajCount);
    skeleton = GWORLD.EntityFromID(skelID).get();
    jointEntityMap.clear();
    for (auto &ele : jointMapSerialize)
      jointEntityMap.insert(
          std::make_pair(ele.first, GWORLD.EntityFromID(ele.second).get()));
    if (actorPath == "none")
      throw std::runtime_error("deserializing an animator without actor");
    else
      actor = Loader.GetActor(actorPath);
    if (motionPath != "none")
      motion = Loader.GetMotion(motionPath);
  }

  // Skeleton visualization related
  // This entity should be the root joint
  Entity *skeleton = nullptr;
  bool ShowSkeleton = true, ShowJoints = false;
  float JointVisualSize = 1.0f;
  bool SkeletonOnTop = false;
  // Color for the visualized skeleton
  glm::vec3 SkeletonColor = glm::vec3(0.0f, 1.0f, 0.0f);

  std::string skeletonName = "";

  // The actor is a read only reference, motion is
  // applied to the skeleton entities created from this actor,
  // the entities could have joint that's not in the actor,
  // but these additional joints won't have overlapping indices with
  // existing actor joints.
  Animation::Skeleton *actor;
  // joint index -> entity of the joint in the scene.
  // if there are joints not defined in actor, additional indices will be
  // assigned
  std::map<std::size_t, Entity *> jointEntityMap;
  // joint index -> whether the joint is active.
  // if there are joints not defined in actor, additional indices will be
  // assigned
  std::map<std::size_t, bool> jointActiveMap;
  // joint name -> joint index.
  // if there are joints not defined in actor, additional indices will be
  // assigned
  std::map<std::string, std::size_t> jointNameToInd;

  // Stores the motion data
  Animation::Motion *motion = nullptr;

  bool ShowTrajectory = false;
  int TrajCount = 3;
  float TrajInterval = 0.2f;

private:
  // only joints defined in actor will be drawn
  void drawSkeletonHierarchy();
  // create skeleton entities from actor, initialize mappings
  void createSkeletonEntities();
};

}; // namespace aEngine