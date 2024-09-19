/**
 * Data structure related to motion data and joints hierarchy,
 * currently, only bvh and fbx motion files are tested.
 *
 * BVH file stores only the offsets of skeleton joints, the initial rotation
 * of all joints are identity rotations. The motion represents the
 * local rotations of joints and translation of root joint.
 *
 * FBX file stores the offsets and rotations of skeleton joints, so we need to
 * apply the initial rotations of a joint to export to bvh, but the motion
 * still represents the local rotation of joints. So we can use the same
 * scheme to construct the skeleton entities and get global positions.
 *
 * Keep in mind, its an easy practice to get local rotation after global
 * rotation.
 */

#pragma once

#include <cmath>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace aEngine {

namespace Animation {

struct Skeleton;
struct Pose;
struct Motion;

// The parent joint has a lower index than all its children
struct Skeleton {
  std::string skeletonName;
  std::vector<std::string> jointNames;
  // local position to joints' parent
  std::vector<glm::vec3> jointOffset;
  // local rotation to joints' parent
  std::vector<glm::quat> jointRotation;
  // local scale to joints' parent
  std::vector<glm::vec3> jointScale;
  // offset matrics for skinning
  std::vector<glm::mat4> offsetMatrices;
  std::vector<int> jointParent;
  std::vector<std::vector<int>> jointChildren;

  // Get the default pose with identity transform on all joints.
  Pose GetRestPose();

  // fbx skeleton need rotation to make up rest post, but bvh don't,
  // handle fbx-style skeleton and bvh-style skeleton export.
  // When a joint with no children is not named following the convention
  // f"{parentName}_End", if the parameter `keepJointNames` is set to true,
  // an `End Site` with offset `0 0 0` will be automatically added.
  // Otherwise, this joint itself will be renamed to `End Site`.
  void ExportAsBVH(std::string filepath, bool keepJointNames = true);

  const int GetNumJoints() { return jointNames.size(); }
};

// By default, the up direction is y-axis (0, 1, 0)
struct Pose {
  Pose() = default;
  ~Pose() {
    skeleton = nullptr;
    jointRotations.clear();
    rootLocalPosition = glm::vec3(0.0f);
  }

  Skeleton *skeleton = nullptr;

  // // local positions of all joints
  // std::vector<glm::vec3> jointPositions;

  // local position for root joint only
  glm::vec3 rootLocalPosition;
  // local rotations of all joints in quaternion
  std::vector<glm::quat> jointRotations;

  // Extract the facing direction projected to xz plane,
  glm::vec3 GetFacingDirection(glm::vec3 restFacing = glm::vec3(0.0f, 0.0f, 1.0f));

  // Perform FK to get global positions for all joints.
  // Mind that `self.ori = parent.ori * self.rot`
  // where `ori` means global rotation, `rot` means local rotation.
  // `self.pos = parent.pos + parent.ori * self.offset`
  // this function works for both fbx-style and bvh-style skeleton
  std::vector<glm::vec3> GetGlobalPositions();

  // Same as `GetGlobalPositions`, but the reference for orientations will be
  // automatically set.
  std::vector<glm::vec3>
  GetGlobalPositionOrientation(std::vector<glm::quat> &orientations);

  // // Get the facing direction on XZ plane
  // glm::vec2 GetFacingDirection();
};

struct Motion {
  Motion() {}
  ~Motion() {}
  int fps;
  Skeleton skeleton;
  std::vector<Pose> poses;

  // We assume that the root's position channels always has the order XYZ,
  // while the rotation channels can have arbitrary orders,
  // the rotation channels can only follow behind the position channels.
  // Be aware that the euler angles in bvh file should
  // be parsed in reversed order, xyz rotation should be quaternion qx*qy*qz.
  bool LoadFromBVH(std::string filename);
  // The saved bvh file's position channels will always be XYZ,
  // the rotation channels will be ZYX and
  // can only follow behind the position channels.
  // Only the root joint has 6 dofs, the rest joints only have 3 dofs.
  // The skeleton and motion will be flatten to offset-only manner
  // automatically.
  // When a joint with no children is not named following the convention
  // f"{parentName}_End", if the parameter `keepJointNames` is set to true,
  // an `End Site` with offset `0 0 0` will be automatically added.
  // Otherwise, this joint itself will be renamed to `End Site`.
  bool SaveToBVH(std::string filename, bool keepJointNames = true);

  // Takes a float value as paramter, returns the slerp interpolated value.
  // If the frame is not valid (out of [0, nframe) range), returns the first
  // frame or last frame respectively.
  Pose At(float frame);
};

}; // namespace Animation

}; // namespace aEngine

#ifdef HEADER_ONLY_MOTION_LIBRARY
#include "Motion.cpp"
#endif