#include "resource/animation/Motion.hpp"

#include "EngineConfig.h"
#include <filesystem>

using namespace std;
namespace fs = std::filesystem;

int main() {
  // string bvhFilename = REPO_SOURCE_DIR "/src/test/animation/test.bvh";
  // Resource::Motion motion;
  // motion.LoadFromBVH(bvhFilename);
  string bvhFileFolder = REPO_SOURCE_DIR "/src/test/animation";
  vector<Resource::Motion> motions;
  for (const auto & entry : fs::directory_iterator(bvhFileFolder)) {
    auto motion = Resource::Motion();
    motion.LoadFromBVH(entry.path().string());
    motions.push_back(std::move(motion));
  }
  return 0;
}