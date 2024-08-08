#include "resource/animation/Motion.hpp"

#include "EngineConfig.h"
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

int main() {
  // string bvhFilename = REPO_SOURCE_DIR "/src/test/animation/zlorp.bvh";
  // Resource::Motion motion;
  // motion.LoadFromBVH(bvhFilename);
  // motion.SaveToBVH("a.bvh");
  string bvhFileFolder = REPO_SOURCE_DIR "/src/test/animation";
  vector<Resource::Motion> motions;
  int index = 0;
  for (const auto & entry : fs::directory_iterator(bvhFileFolder)) {
    auto motion = Resource::Motion();
    motion.LoadFromBVH(entry.path().string());
    motions.push_back(motion);
    motion.SaveToBVH(std::to_string(index++)+".bvh");
  }
  return 0;
}