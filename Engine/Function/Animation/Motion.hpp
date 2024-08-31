// Data structure related to motion data and joints hierarchy

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
  // offset matrics needed for skinning
  std::vector<glm::mat4> offsetMatrices;
  std::vector<int> jointParent;
  std::vector<std::vector<int>> jointChildren;

  // Get the default pose with identity transform on all joints.
  Pose GetRestPose();

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

  // Perform FK to get global positions for all joints.
  // Mind that `self.ori = parent.ori * self.rot`
  // where `ori` means global rotation, `rot` means local rotation.
  std::vector<glm::vec3> GetGlobalPositions();

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
  bool SaveToBVH(std::string filename);

  // Takes a float value as paramter, returns the slerp interpolated value.
  // If the frame is not valid (<0 or >nframes), returns the first frame or last
  // frame respectively.
  Pose At(float frame);
};

}; // namespace Animation

}; // namespace aEngine

#ifdef HEADER_ONLY_MOTION_LIBRARY
#include "Motion.cpp"
#endif