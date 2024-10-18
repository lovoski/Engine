#include "Scripts/Animation/MotionMatching.hpp"

#include "Function/GUI/Helpers.hpp"
#include "Function/Math/Dampers.hpp"

namespace aEngine {

void buildMotionDatabase(std::string filepath, MotionDatabase &db, int lfoot,
                         int rfoot, int hip);
void loadMotionDatabase(std::string filepath, MotionDatabase &db);
void saveMotionDatabase(std::string filepath, MotionDatabase &db);

void MotionMatching::Update(float dt) {
  // joystick related
  queryJoysticks();
  if (currentJoystick != -1) {
    int axisCount;
    const float *axes = glfwGetJoystickAxes(currentJoystick, &axisCount);
    leftInput = glm::vec2(axes[0], axes[1]);
    rightInput = glm::vec2(axes[2], axes[3]);
  } else {
    leftInput = glm::vec2(0.0f);
    rightInput = glm::vec2(0.0f);
  }
  // update player position and rotation
  glm::vec3 forward = glm::normalize(playerFacing);
  glm::vec3 right = glm::normalize(glm::cross(Entity::WorldUp, forward));
  glm::vec3 desiredSpeed =
      -playerSpeed * (leftInput.y * forward + leftInput.x * right);

  bool hasLeftInput = glm::length(leftInput) > 1e-3f;
  bool hasRightInput = glm::length(rightInput) > 1e-3f;
  if (hasRightInput) {
    glm::quat rotY = glm::angleAxis(-4.0f * dt * rightInput.x, Entity::WorldUp);
    playerFacing = rotY * playerFacing;
    playerFacing.y = 0.0f;
  }

  Math::DamperExp(speed, desiredSpeed, dt, speedHalfLife);
  playerPosition += dt * speed;

  // update trajectory
  trajPos.clear();
  trajSpeed.clear();
  trajPos.resize(trajCount, playerPosition);
  trajSpeed.resize(trajCount, speed);
  for (int i = 1; i < trajCount; ++i) {
    Math::DamperExp(trajSpeed[i], desiredSpeed, trajInterval * i,
                    speedHalfLife);
    trajPos[i] = 0.5f * (trajSpeed[i] + trajSpeed[i - 1]) * trajInterval +
                 trajPos[i - 1];
  }
}

void MotionMatching::LateUpdate(float dt) {
  // database query, update animator motion on a fixed frequency
  auto animator = entity->GetComponent<Animator>();
  if (animator != nullptr && database.features.size() > 0) {
    elapsedTime += dt;
    if (elapsedTime >= fixedUpdateTime) {
      while (elapsedTime >= fixedUpdateTime) {
        updateAnimatorMotion(animator);
        elapsedTime -= fixedUpdateTime;
      }
      elapsedTime = 0.0f;
    }
  }

  // camera adjustment
  if (orbitCamera) {
    EntityID camera;
    if (GWORLD.GetActiveCamera(camera)) {
      auto cameraObject = GWORLD.EntityFromID(camera);
      // re-pose the camera
      // camForward = -camFacing
      glm::vec3 camLeft =
          glm::normalize(glm::cross(Entity::WorldUp, -cameraFacing));
      Math::DamperExp(cameraFacing, playerFacing, dt, 0.5f);
      glm::vec3 offsetDir =
          glm::angleAxis(glm::radians(cameraAngle), camLeft) * cameraFacing;
      cameraObject->SetGlobalPosition(playerPosition -
                                      cameraOffset * glm::normalize(offsetDir));
      glm::vec3 rotForward = glm::normalize(-offsetDir);
      glm::vec3 rotLeft = camLeft;
      glm::vec3 rotUp = glm::normalize(glm::cross(rotForward, rotLeft));
      cameraObject->SetGlobalRotation(
          glm::quat_cast(glm::mat3(rotLeft, rotUp, rotForward)));
    }
  }
}

void MotionMatching::updateAnimatorMotion(std::shared_ptr<Animator> &animator) {
  if (lastRotation.empty()) {
    lastRotation.resize(animator->jointEntityMap.size());
    for (int i = 0; i < animator->jointEntityMap.size(); ++i)
      lastRotation[i] = animator->jointEntityMap[i]->Rotation();
  }
  if (deltaRotation.empty())
    deltaRotation.resize(animator->jointEntityMap.size(),
                         glm::quat(1.0f, glm::vec3(0.0f)));
  searchFrameCounter--;
  if (searchFrameCounter <= 0 ||
      (currentFrameInd + 1) >= database.data.size()) {
    int bestFrameInd = currentFrameInd + 1;
    glm::vec3 hipvel = database.data[bestFrameInd].velocities[hipIndex];
    glm::vec3 hippos = database.data[bestFrameInd].positions[hipIndex];
    std::array<glm::vec3, 2> lfootData{
        database.data[bestFrameInd].positions[lfootIndex] - hippos,
        database.data[bestFrameInd].velocities[lfootIndex]};
    std::array<glm::vec3, 2> rfootData{
        database.data[bestFrameInd].positions[rfootIndex] - hippos,
        database.data[bestFrameInd].velocities[rfootIndex]};
    std::array<glm::vec2, 4> trajData, facingDir;
    trajData[0] = glm::vec2(0.0f);
    auto playerPosProj = glm::vec2(playerPosition.x, playerPosition.z);
    // trajectory relative to root position
    for (int i = 1; i <= 3; ++i)
      trajData[i] = glm::vec2(trajPos[i].x, trajPos[i].z) - playerPosProj;
    // facing directions
    for (int i = 0; i < 4; ++i)
      facingDir[i] = glm::vec2(playerFacing.x, playerFacing.z);
    // compress into a feature
    auto currentFeature = database.CompressFeature(hipvel, lfootData, rfootData,
                                                   trajData, facingDir);
    for (int k = 0; k < database.featureMean.max_size(); ++k)
      currentFeature[k] = (currentFeature[k] - database.featureMean[k]) /
                          database.featureStd[k];
    // query motion database for closest feature
    currentFrameInd = tree.BruteForceNearestSearch(currentFeature);
    // update delta rotations
    for (int i = 0; i < animator->jointEntityMap.size(); ++i) {
      auto lrot = lastRotation[i];
      auto crot = animator->jointEntityMap[i]->Rotation();
      deltaRotation[i] = crot * glm::inverse(lrot);
    }
    // reset counter
    searchFrameCounter = searchFrame;
    crossFadeDeltaCount = 0;
  } else {
    currentFrameInd++;
    crossFadeDeltaCount++;
  }

  // update character motion, make transition
  auto motionData = database.data[currentFrameInd];
  for (int jointInd = 0; jointInd < animator->jointEntityMap.size(); ++jointInd) {
    auto jointEntity = animator->jointEntityMap[jointInd];
    auto currentRot = jointEntity->Rotation();
    // update last rotation
    lastRotation[jointInd] = currentRot;

    auto nextRot = motionData.rotations[jointInd];
    // make sure these rotations are in the same hemisphere
    if (glm::dot(currentRot, nextRot) < 0.0f)
      nextRot *= -1;

    // utilize delta rotation to blend the motion
    float alpha = (float)crossFadeDeltaCount / (float)crossFadeWndSize;
    if (alpha < 1.0f) {
      currentRot =
          glm::normalize((1.0f - alpha) * currentRot + alpha * nextRot);
    } else
      currentRot = nextRot;

    if (jointInd == 0) {
      glm::vec3 currentPos = glm::vec3(
          playerPosition.x, motionData.positions[jointInd].y, playerPosition.z);
      jointEntity->SetGlobalPosition(currentPos);
    }
    jointEntity->SetGlobalRotation(currentRot);
  }
}

void MotionMatching::DrawToScene() {
  EntityID camera;
  if (GWORLD.GetActiveCamera(camera)) {
    auto cameraComp = GWORLD.GetComponent<Camera>(camera);
    auto vp = cameraComp->VP;
    VisUtils::DrawArrow(playerPosition, playerPosition + playerFacing, vp,
                        VisUtils::Blue);
    VisUtils::DrawLineStrip3D(trajPos, vp, VisUtils::Red);
    for (int i = 0; i < trajCount; ++i) {
      VisUtils::DrawWireSphere(trajPos[i], vp, 0.1f, VisUtils::Red);
      VisUtils::DrawArrow(trajPos[i], trajPos[i] + 0.5f * trajSpeed[i], vp,
                          VisUtils::Yellow, 0.05f);
    }
  }
}

void MotionMatching::DrawInspectorGUI() {
  ImGui::MenuItem("Build Database", nullptr, nullptr, false);
  static std::vector<char> sourceDirBuffer(200);
  GUIUtils::DragableFileTarget(
      "motionmatchingsourcedir", "Directory Path",
      [&](std::string path) {
        // TODO: make it modifiable in inspector gui
        buildMotionDatabase(path, database, lfootIndex, rfootIndex, hipIndex);
        tree.Build(database.features);
        return true;
      },
      sourceDirBuffer);

  ImGui::MenuItem("Load Database", nullptr, nullptr, false);
  static std::vector<char> databaseFile(200);
  GUIUtils::DragableFileTarget(
      "motionmatchingdatabase", "Database File (data.bin)",
      [&](std::string file) {
        if (fs::path(file).filename().string() == "data.bin") {
          loadMotionDatabase(file, database);
          tree.Build(database.features);
          return true;
        }
        return false;
      },
      databaseFile);

  ImGui::MenuItem("User Input", nullptr, nullptr, false);
  GUIUtils::Combo("Joysticks", joystickNames, _cjInd, [&](int current) {
    if (current == -1)
      currentJoystick = -1;
    else
      currentJoystick = availableJoysticks[current];
  });

  ImGui::MenuItem("Camera", nullptr, nullptr, false);
  ImGui::Checkbox("Orbit Camera", &orbitCamera);
  ImGui::DragFloat("Camera Offset", &cameraOffset, 0.2f, 0.0f, 1000.0f);
  ImGui::SliderFloat("Camera Angle", &cameraAngle, -80.0f, -20.0f);
  ImGui::SliderFloat("Camera Sensitivity", &cameraSensitivity, 0.0f, 10.0f);
  ImGui::MenuItem("Player", nullptr, nullptr, false);
  ImGui::SliderFloat("Player Speed", &playerSpeed, 0.5f, 10.0f);
  ImGui::SliderFloat("Speed Half Life", &speedHalfLife, 0.0f, 3.0f);

  // ImGui::MenuItem("Trajectory", nullptr, nullptr, false);
  // ImGui::SliderInt("Trajectory Count", &trajCount, 1, 9);
  // ImGui::SliderFloat("Trajectory Interval", &trajInterval, 0.1f, 1.0f);

  ImGui::SliderInt("Cross Fade Window Size", &crossFadeWndSize, 10, 50);
  ImGui::SliderInt("Search Frames", &searchFrame, 1, 60);
}

// ---------------------- Helper functions ----------------------

void saveMotionDatabase(std::string filepath, MotionDatabase &db) {
  std::ofstream output(filepath, std::ios::binary);
  cereal::PortableBinaryOutputArchive oa(output);
  oa(db);
}

void loadMotionDatabase(std::string filepath, MotionDatabase &db) {
  std::ifstream input(filepath, std::ios::binary);
  cereal::PortableBinaryInputArchive ia(input);
  ia(db);
}

void buildMotionDatabase(std::string filepath, MotionDatabase &db, int lfoot,
                         int rfoot, int hip) {
  // clear the database
  db.features.clear();
  db.range.clear();
  db.data.clear();
  // load motion data from the filepath
  auto processMotionData = [&](std::string file) {
    Animation::Motion motion;
    motion.LoadFromBVH(file);
    std::vector<glm::vec3> lastPositions;
    std::vector<glm::quat> lastRotations;
    int start = db.data.size();
    for (auto &pose : motion.poses) {
      MotionDatabaseData mdd;
      mdd.facingDir = pose.GetFacingDirection();
      mdd.positions = pose.GetGlobalPositionOrientation(mdd.rotations);
      mdd.velocities.resize(mdd.positions.size(), glm::vec3(0.0f));
      mdd.angularVel.resize(mdd.positions.size(), glm::vec3(0.0f));
      if (!lastPositions.empty()) {
        for (int i = 0; i < mdd.positions.size(); ++i) {
          mdd.velocities[i] =
              (mdd.positions[i] - lastPositions[i]) / (float)motion.fps;
        }
      }
      if (!lastRotations.empty()) {
        for (int i = 0; i < mdd.positions.size(); ++i) {
          auto delta = mdd.rotations[i] * glm::inverse(lastRotations[i]);
          mdd.angularVel[i] =
              2.0f * motion.fps * glm::vec3(delta.x, delta.y, delta.z);
        }
      }
      db.data.push_back(mdd);
      lastPositions = mdd.positions;
      lastRotations = mdd.rotations;
    }
    int end = db.data.size();
    db.dataFPS = motion.fps;
    db.range.push_back(std::make_pair(start, end));
  };
  if (fs::exists(filepath) && fs::is_directory(filepath)) {
    for (const auto &entry : fs::recursive_directory_iterator(filepath)) {
      if (fs::is_regular_file(entry.path()) &&
          entry.path().extension().string() == ".bvh") {
        processMotionData(entry.path().string());
      }
    }
    // calculate motion feature
    db.ComputeFeatures(lfoot, rfoot, hip);
    saveMotionDatabase((fs::path(filepath) / fs::path("data.bin")).string(),
                       db);
  } else {
    LOG_F(ERROR, "%s is not a directory", filepath.c_str());
  }
}

void MotionDatabase::ComputeFeatures(int lfoot, int rfoot, int hip) {
  int interval = trajInterval * dataFPS;
  featureMean.fill(0.0f);
  for (int animInd = 0; animInd < range.size(); ++animInd) {
    int start = range[animInd].first;
    int end = range[animInd].second;
    for (int f = start; f < end; ++f) {
      glm::vec3 hipVel = data[f].velocities[hip];
      std::array<glm::vec3, 2> lfootData, rfootData;
      lfootData[0] = data[f].positions[lfoot] - data[f].positions[hip];
      lfootData[1] = data[f].velocities[lfoot];
      rfootData[0] = data[f].positions[rfoot] - data[f].positions[hip];
      rfootData[1] = data[f].velocities[rfoot];
      std::array<glm::vec2, 4> trajData;
      std::array<glm::vec2, 4> facingDir;
      // sample trajectories
      auto rootPosProj =
          glm::vec2(data[f].positions[hip].x, data[f].positions[hip].z);
      trajData[0] = glm::vec2(0.0f);
      facingDir[0] = data[f].facingDir;
      for (int i = 1; i <= 4; ++i) {
        int sampleF =
            ((end - 1) < (f + i * interval)) ? (end - 1) : (f + i * interval);
        glm::vec3 sampleHip = data[sampleF].positions[hip];
        trajData[i - 1] = glm::vec2(sampleHip.x, sampleHip.z) - rootPosProj;
      }
      for (int i = 1; i <= 4; ++i) {
        int sampleF =
            ((end - 1) < (f + i * interval)) ? (end - 1) : (f + i * interval);
        glm::vec3 sampleFacingDir = data[sampleF].facingDir;
        facingDir[i - 1] = glm::vec2(sampleFacingDir.x, sampleFacingDir.z);
      }
      auto newFeature =
          CompressFeature(hipVel, lfootData, rfootData, trajData, facingDir);
      for (int k = 0; k < newFeature.max_size(); ++k)
        featureMean[k] += newFeature[k];
      features.push_back(newFeature);
    }
  }
  for (int i = 0; i < featureMean.max_size(); ++i)
    featureMean[i] /= features.size();
  featureStd.fill(0.0f);
  for (int i = 0; i < features.size(); ++i)
    for (int k = 0; k < featureMean.max_size(); ++k)
      featureStd[k] +=
          (features[i][k] - featureMean[k]) * (features[i][k] - featureMean[k]);
  for (int i = 0; i < featureStd.max_size(); ++i)
    featureStd[i] = std::sqrt(featureStd[i] / features.size());
  for (int i = 0; i < features.size(); ++i)
    for (int k = 0; k < featureMean.max_size(); ++k)
      features[i][k] = (features[i][k] - featureMean[k]) / featureStd[k];
}

std::array<float, 31> MotionDatabase::CompressFeature(
    glm::vec3 &hipvel, std::array<glm::vec3, 2> &lfoot,
    std::array<glm::vec3, 2> &rfoot, std::array<glm::vec2, 4> &traj,
    std::array<glm::vec2, 4> &facingDir) {
  std::array<float, 31> feature;
  int index = 0;
  for (int i = 0; i < 3; ++i)
    feature[index++] = hipvel[i];
  for (int i = 0; i < 2; ++i)
    for (int j = 0; j < 3; ++j)
      feature[index++] = lfoot[i][j];
  for (int i = 0; i < 2; ++i)
    for (int j = 0; j < 3; ++j)
      feature[index++] = rfoot[i][j];
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 2; ++j)
      feature[index++] = traj[i][j];
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 2; ++j)
      feature[index++] = facingDir[i][j];
  return feature;
}

void MotionMatching::queryJoysticks() {
  availableJoysticks.clear();
  joystickNames.clear();
  for (int jid = GLFW_JOYSTICK_1; jid < GLFW_JOYSTICK_LAST; ++jid) {
    if (glfwJoystickPresent(jid)) {
      availableJoysticks.push_back(jid);
      joystickNames.push_back(glfwGetJoystickName(jid));
    }
  }
}

}; // namespace aEngine

REGISTER_SCRIPT(aEngine, MotionMatching)