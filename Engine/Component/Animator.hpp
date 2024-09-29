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

struct BoneMatrixBlock {
  glm::mat4 BoneModelMatrix;
  glm::mat4 BoneOffsetMatrix;
};

// Each animator must bind to one actor (Skeleton),
// the entity structure will be created when one animator gets created
struct Animator : public BaseComponent {
  Animator() : BaseComponent(-1) {}
  Animator(EntityID id, Animation::Skeleton *act);
  Animator(EntityID id, Animation::Motion *m);
  ~Animator();

  void DrawInspectorGUI() override;

  // Get transformation matrics needed for skeleton animation
  std::vector<BoneMatrixBlock> GetSkeletonTransforms();

  // Apply the motion to skeleton entities
  void ApplyMotionToSkeleton(Animation::Pose &pose);
  // Maintain member variable `SkeletonMap`, the key for this map is the name
  // of the entity.
  // The function is rather costly, only call it when the skeleton entity
  // hierarchy is actually modified.
  void BuildSkeletonMap();

  std::string getInspectorWindowName() override { return "Animator"; }

  template <typename Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &boost::serialization::base_object<BaseComponent>(*this);
    if (typename Archive::is_saving()) {
      EntityID id = skeleton == nullptr ? (EntityID)(-1) : skeleton->ID;
      ar &id;
    } else if (typename Archive::is_loading()) {
      EntityID id;
      ar &id;
      if (id != (EntityID)(-1))
        skeleton = nullptr;
      else
        skeleton = GWORLD.EntityFromID(id).get();
    }
    ar &ShowSkeleton;
    ar &ShowJoints;
    ar &SkeletonOnTop;
    ar &SkeletonColor;
    ar &skeletonName;
    ar &motionName;
    ar &jointActive;
    // TODO: actor serialization
    ar &actorJointMap;
    // motion serialization
  }

  // Skeleton visualization related
  // This entity should be the root joint
  Entity *skeleton = nullptr;
  bool ShowSkeleton = true, ShowJoints = false;
  float JointVisualSize = 1.0f;
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
  // map actor's jointName hash to joint index, this is a static structure
  // and won't get updated with skeleton entities.
  // To get the hash of the string, use `HashString("xxx")`
  std::map<std::size_t, int> actorJointMap;
  // This map should have the same size as the actor's joints,
  // map the hash of jointName to skeleton data
  std::map<std::size_t, SkeletonMapData> SkeletonMap;

  // Stores the motion data
  Animation::Motion *motion = nullptr;

private:
  void drawSkeletonHierarchy();
  void createSkeletonEntities();
};

}; // namespace aEngine