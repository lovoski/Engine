#pragma once

#include <cmath>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Resource {

#define EULER_XYZ 12
#define EULER_XZY 21
#define EULER_YXZ 102
#define EULER_YZX 120
#define EULER_ZXY 201
#define EULER_ZYX 210

struct Skeleton {
  std::string skeletonName;
  // the parent joint should always have a lower index than its children
  std::vector<std::string> jointNames;
  std::vector<glm::vec3> jointOffset;
  std::vector<int> jointParent;
  std::vector<std::vector<int>> jointChildren;

  // for bvh file format only
  std::vector<int> jointChannels;
  std::vector<int> jointChannelsOrder;

  int GetNumJoints() { return jointNames.size(); }
};

struct Pose {
  Skeleton *skeleton = nullptr;

  // these arrays should have the size of skeleton.GetNumJoints()

  // local positions of all joints
  std::vector<glm::vec3> jointPositions;
  // local rotations of all joints in euler angles
  std::vector<glm::vec3> jointRotations;

  std::vector<glm::vec3> GetGlobalPositions() {}
  std::vector<glm::quat> GetGlobalRotations() {}
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
  bool LoadFromBVH(std::string filename);
  // The saved bvh file's position channels will always be XYZ, 
  // the rotation channels will have the same order as it the loaded file and 
  // can only follow behind the position channels.
  bool SaveToBVH(std::string filename);
};

}; // namespace Resource

#ifdef HEADER_ONLY_MOTION_LIBRARY
#include "Motion.cpp"
#endif