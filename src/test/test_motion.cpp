#include "resource/animation/Motion.hpp"

#include "EngineConfig.h"
#include <filesystem>

#include <iostream>

using namespace std;
namespace fs = std::filesystem;

int main() {
  string bvhFilename = REPO_SOURCE_DIR "/src/test/animation/motion/LocomotionFlat01_000_2.bvh";
  Resource::Motion motion;
  motion.LoadFromBVH(bvhFilename);
  motion.SaveToBVH("a.bvh");

  // glm::vec3 eulerZXY = glm::radians(glm::vec3(3.36719 - 6.07025 - 3.68531));
  // glm::quat q = glm::angleAxis(eulerZXY.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
  //               glm::angleAxis(eulerZXY.x, glm::vec3(1.0f, 0.0f, 0.0f)) *
  //               glm::angleAxis(eulerZXY.z, glm::vec3(0.0f, 0.0f, 1.0f));
  // glm::vec3 eulerXYZ = glm::eulerAngles(q);
  // glm::quat q1 = glm::angleAxis(eulerXYZ.z, glm::vec3(0.0f, 0.0f, 1.0f)) *
  //                glm::angleAxis(eulerXYZ.y, glm::vec3(0.0f, 1.0f, 0.0f)) *
  //                glm::angleAxis(eulerXYZ.x, glm::vec3(1.0f, 0.0f, 0.0f));

  // glm::vec3 v(100, 20, 0);
  // auto v1 = q * v;
  // auto v2 = q1 * v;
  // cout << v1.x << " " << v1.y << " " << v1.z << endl;
  // cout << v2.x << " " << v2.y << " " << v2.z << endl;

  // cout << q.x << " " << q.y << " " << q.z << " " << q.w << endl;
  // cout << q1.x << " " << q1.y << " " << q1.z << " " << q1.w << endl;

  // string bvhFileFolder = REPO_SOURCE_DIR "/src/test/animation";
  // vector<Resource::Motion> motions;
  // int index = 0;
  // for (const auto & entry : fs::directory_iterator(bvhFileFolder)) {
  //   auto motion = Resource::Motion();
  //   motion.LoadFromBVH(entry.path().string());
  //   motions.push_back(motion);
  //   motion.SaveToBVH(std::to_string(index++)+".bvh");
  // }
  return 0;
}