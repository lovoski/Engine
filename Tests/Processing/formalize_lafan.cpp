/**
 * Formalize mocap data from lafan dataset:
 * https://github.com/ubisoft/ubisoft-laforge-animation-dataset
 */
#include "Function/Animation/Edit.hpp"
#include "Function/Animation/Motion.hpp"

#include "EngineConfig.h"

using namespace std;
using namespace aEngine;
using namespace aEngine::Animation;

void processMotionData(string filepath, string prefix) {
  fs::path outputPath = fs::path(prefix) / fs::path(filepath).filename();
  auto motion = Motion();
  motion.LoadFromBVH(filepath, 0.01f);
  // 1. remove root translation in skeleton
  auto skelRootTranslation = motion.skeleton.jointOffset[0];
  skelRootTranslation.y = 0;
  motion.skeleton.jointOffset[0] = glm::vec3(0.0f);
  for (auto &pose : motion.poses)
    pose.rootLocalPosition -= skelRootTranslation;
  // 2. translate the first frame to origin, and set as rest pose
  auto restPose = motion.poses[0];
  for (int i = 0; i < motion.skeleton.GetNumJoints(); ++i)
    motion.skeleton.jointRotation[i] = restPose.jointRotations[i];
  // 3. write processed motion data to a new localtion
  motion.SaveToBVH(outputPath.string());
  cout << "write motion to " << outputPath.string() << endl;
}

int main() {
  string lafanMocapDir = ASSETS_PATH "/../../build/Debug/lafan",
         outputMocapDir = ASSETS_PATH "/../../build/Debug/lafan_processed";
  if (fs::create_directory(outputMocapDir)) {
    for (const auto &entry : fs::recursive_directory_iterator(lafanMocapDir)) {
      if (fs::is_regular_file(entry.path()) &&
          entry.path().extension().string() == ".bvh") {
        processMotionData(entry.path().string(), outputMocapDir);
      }
    }
  } else {
    cout << "path: " << outputMocapDir
         << " already exists or directory creation failed" << endl;
  }
  return 0;
}