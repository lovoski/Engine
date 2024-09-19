#include "Scripts/Animation/MotionMatching.hpp"

#include "Function/GUI/Helpers.hpp"

#include <cereal/archives/binary.hpp>
#include <cereal/types/vector.hpp>

namespace aEngine {

void buildMotionDatabase(std::string filepath);
void loadMotionDatabase(std::string filepath,
                        std::vector<MotionDatabaseData> &data);
void saveMotionDatabase(std::string filepath,
                        std::vector<MotionDatabaseData> &data);

MotionMatching::MotionMatching() {}

MotionMatching::~MotionMatching() {}

void MotionMatching::Start() { LOG_F(WARNING, "start"); }
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
  // update player position
  glm::vec3 speed{leftInput.x, 0.0f, leftInput.y};
  playerPosition += dt * speed;
}

void MotionMatching::LateUpdate(float dt) {
  // database query
  if (auto animator = entity->GetComponent<Animator>()) {
  }
  // camera adjustment
}

void MotionMatching::DrawToScene() {
  EntityID camera;
  if (GWORLD.GetActiveCamera(camera)) {
    auto cameraComp = GWORLD.GetComponent<Camera>(camera);
    auto vp = cameraComp->VP;
    VisUtils::DrawWireSphere(playerPosition, vp);
    VisUtils::DrawArrow(playerPosition,
                        playerPosition + glm::vec3(leftInput.x, 0.0f, leftInput.y), vp,
                        glm::vec3(1.0f, 0.0f, 0.0f));
    VisUtils::DrawArrow(playerPosition,
                        playerPosition + glm::vec3(rightInput.x, 0.0f, rightInput.y), vp,
                        glm::vec3(0.0f, 0.0f, 1.0f));
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
          std::vector<MotionDatabaseData> data;
          loadMotionDatabase(file, data);
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

void saveMotionDatabase(std::string filepath,
                        std::vector<MotionDatabaseData> &data) {
  std::ofstream output(filepath, std::ios::binary);
  cereal::BinaryOutputArchive archive(output);
  archive(data);
}

void loadMotionDatabase(std::string filepath,
                        std::vector<MotionDatabaseData> &data) {
  std::ifstream input(filepath, std::ios::binary);
  cereal::BinaryInputArchive archive(input);
  data.clear();
  archive(data);
}

void buildMotionDatabase(std::string filepath) {
  // load motion data from the filepath
  std::vector<MotionDatabaseData> data;
  auto processMotionData = [&](std::string file) {
    Animation::Motion motion;
    motion.LoadFromBVH(file);
    std::vector<glm::vec3> lastPositions;
    for (auto &pose : motion.poses) {
      MotionDatabaseData mdd;
      mdd.facingDir = pose.GetFacingDirection();
      mdd.positions = pose.GetGlobalPositions();
      mdd.velocities.resize(mdd.positions.size(), glm::vec3(0.0f));
      if (!lastPositions.empty()) {
        for (int i = 0; i < mdd.positions.size(); ++i) {
          mdd.velocities[i] = mdd.positions[i] - lastPositions[i];
        }
      }
      data.push_back(mdd);
      lastPositions = mdd.positions;
    }
  };
  if (fs::exists(filepath) && fs::is_directory(filepath)) {
    for (const auto &entry : fs::recursive_directory_iterator(filepath)) {
      if (fs::is_regular_file(entry.path()) &&
          entry.path().extension().string() == ".bvh") {
        processMotionData(entry.path().string());
      }
    }
    saveMotionDatabase((fs::path(filepath) / fs::path("data.bin")).string(),
                       data);
  } else {
    LOG_F(ERROR, "%s is not a directory", filepath.c_str());
  }
}

}; // namespace aEngine