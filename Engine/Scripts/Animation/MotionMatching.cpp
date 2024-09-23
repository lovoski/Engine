#include "Scripts/Animation/MotionMatching.hpp"

#include "Function/GUI/Helpers.hpp"
#include "Function/Math/Dampers.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/types/common.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>

namespace aEngine {

void buildMotionDatabase(std::string filepath);
void loadMotionDatabase(std::string filepath, MotionDatabase &db);
void saveMotionDatabase(std::string filepath, MotionDatabase &db);

MotionMatching::MotionMatching() {}

MotionMatching::~MotionMatching() {}

void MotionMatching::OnEnable() {}
void MotionMatching::OnDisable() {}

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
    trajPos[i] = 0.5f * (trajSpeed[i] + trajSpeed[i - 1]) + trajPos[i - 1];
  }
}

void MotionMatching::LateUpdate(float dt) {
  // database query
  if (auto animator = entity->GetComponent<Animator>()) {
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

void MotionMatching::DrawToScene() {
  EntityID camera;
  if (GWORLD.GetActiveCamera(camera)) {
    auto cameraComp = GWORLD.GetComponent<Camera>(camera);
    auto vp = cameraComp->VP;
    VisUtils::DrawWireSphere(playerPosition, vp);
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
        buildMotionDatabase(path);
        return true;
      },
      sourceDirBuffer);

  ImGui::MenuItem("Load Database", nullptr, nullptr, false);
  static std::vector<char> databaseFile(200);
  GUIUtils::DragableFileTarget(
      "motionmatchingdatabase", "Database File (data.bin)",
      [&](std::string file) {
        if (fs::path(file).filename().string() == "data.bin") {
          MotionDatabase db;
          loadMotionDatabase(file, db);
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
  ImGui::MenuItem("Trajectory", nullptr, nullptr, false);
  ImGui::SliderInt("Trajectory Count", &trajCount, 1, 9);
  ImGui::SliderFloat("Trajectory Interval", &trajInterval, 0.1f, 1.0f);
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

void saveMotionDatabase(std::string filepath, MotionDatabase &db) {
  std::ofstream output(filepath, std::ios::binary);
  cereal::BinaryOutputArchive archive(output);
  archive(db);
}

void loadMotionDatabase(std::string filepath, MotionDatabase &db) {
  std::ifstream input(filepath, std::ios::binary);
  cereal::BinaryInputArchive archive(input);
  archive(db);
}

void buildMotionDatabase(std::string filepath) {
  // load motion data from the filepath
  MotionDatabase db;
  auto processMotionData = [&](std::string file) {
    Animation::Motion motion;
    motion.LoadFromBVH(file);
    std::vector<glm::vec3> lastPositions;
    int start = db.data.size();
    for (auto &pose : motion.poses) {
      MotionDatabaseData mdd;
      mdd.facingDir = pose.GetFacingDirection();
      mdd.positions = pose.GetGlobalPositionOrientation(mdd.rotations);
      mdd.velocities.resize(mdd.positions.size(), glm::vec3(0.0f));
      if (!lastPositions.empty()) {
        for (int i = 0; i < mdd.positions.size(); ++i) {
          mdd.velocities[i] = mdd.positions[i] - lastPositions[i];
        }
      }
      db.data.push_back(mdd);
      lastPositions = mdd.positions;
    }
    int end = db.data.size();
    db.range.push_back(std::make_pair(start, end));
  };
  if (fs::exists(filepath) && fs::is_directory(filepath)) {
    for (const auto &entry : fs::recursive_directory_iterator(filepath)) {
      if (fs::is_regular_file(entry.path()) &&
          entry.path().extension().string() == ".bvh") {
        processMotionData(entry.path().string());
      }
    }
    saveMotionDatabase((fs::path(filepath) / fs::path("data.bin")).string(),
                       db);
  } else {
    LOG_F(ERROR, "%s is not a directory", filepath.c_str());
  }
}

void MotionDatabase::Query() {}

}; // namespace aEngine