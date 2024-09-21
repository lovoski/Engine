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

void MotionMatching::OnEnable() { LOG_F(WARNING, "onenable"); }
void MotionMatching::OnDisable() { LOG_F(WARNING, "ondisable"); }

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
  // update player position and facing direction
  glm::vec3 speed{leftInput.x, 0.0f, leftInput.y};
  if (glm::length(rightInput) > 1e-3f) {
    auto normalizedFacing = glm::normalize(rightInput);
    playerFacing = glm::vec3(normalizedFacing.x, 0.0f, normalizedFacing.y);
  }
  playerPosition += dt * speed;
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
    }
  }

  // damper test
  Math::DamperSpring(damperPosition, damperVelocity, entity->Position(),
                     glm::vec3(0.0f), dt, stiffness, damping);
  ;
}

void MotionMatching::DrawToScene() {
  EntityID camera;
  if (GWORLD.GetActiveCamera(camera)) {
    auto cameraComp = GWORLD.GetComponent<Camera>(camera);
    auto vp = cameraComp->VP;
    VisUtils::DrawWireSphere(playerPosition, vp);
    VisUtils::DrawArrow(playerPosition,
                        playerPosition +
                            glm::vec3(leftInput.x, 0.0f, leftInput.y),
                        vp, glm::vec3(1.0f, 0.0f, 0.0f));
    VisUtils::DrawArrow(playerPosition, playerPosition + playerFacing, vp,
                        glm::vec3(0.0f, 0.0f, 1.0f));

    VisUtils::DrawWireSphere(damperPosition, vp);
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

  ImGui::MenuItem("Options", nullptr, nullptr, false);
  ImGui::Checkbox("Orbit Camera", &orbitCamera);

  ImGui::MenuItem("Damper", nullptr, nullptr, false);
  ImGui::SliderFloat("Damping", &damping, 0.0f, 40.0f);
  ImGui::SliderFloat("Stiffness", &stiffness, 0.0f, 40.0f);
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

}; // namespace aEngine