#pragma once

#include <vector>
#include <string>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Resource {

// enum CHANNEL_AXIS_ORDER {
//   XYZ, XZY, YXZ, YZX, ZXY, ZYX, NONE
// };

struct Skeleton {
  std::string skeletonName;
  // the parent joint should always have a lower index than its children
  std::vector<std::string> jointNames;
  std::vector<glm::vec3> jointOffset;
  std::vector<int> jointParent;
  std::vector<std::vector<int>> jointChildren;

  // // for bvh format only
  // std::vector<int> jointChannels;
  // // for bvh format only
  // std::vector<CHANNEL_AXIS_ORDER> jointChannelsType;

  int GetNumJoints() { return jointNames.size(); }
};

struct Pose {
  Skeleton *skeleton = nullptr;
  // these arrays should have the size of skeleton.GetNumJoints()
  std::vector<glm::vec3> jointPositions;
  std::vector<glm::quat> jointRotations;

  std::vector<glm::vec3> GetGlobalPositions() {}
  std::vector<glm::quat> GetGlobalRotations() {}
};

struct Motion {
  Motion() {}
  ~Motion() {}
  int fps;
  Skeleton skeleton;
  std::vector<Pose> poses;

  bool LoadFromBVH(std::string filename);
  bool SaveToBVH(std::string filename);
};

}; // namespace Resource

#ifdef HEADER_ONLY_MOTION_LIBRARY
#include "Motion.cpp"
#endif