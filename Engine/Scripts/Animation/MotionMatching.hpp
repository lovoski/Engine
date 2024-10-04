/**
 * Thanks to: https://theorangeduck.com/page/code-vs-data-driven-displacement
 *
 * This is a implementation to a character controller based on motion matching,
 * attach this script to some entity with `Animator` component, setup the
 * motion database to use. Then you should be free to control the character.
 *
 * The idea of motion matching is to precompute a database of features for all
 * motion clips, construct a query feature from user input and previous motion,
 * select a motion clip with closest feature to the user input and play it for
 * a while. After playing current clip for a while, perform another query and
 * decide whether to switch current animation clip or not.
 */
#pragma once

#include "API.hpp"

namespace aEngine {

struct MotionDatabaseData {
  glm::vec3 facingDir = glm::vec3(0.0f);
  // global positions
  std::vector<glm::vec3> positions;
  // global orientations
  std::vector<glm::quat> rotations;
  std::vector<glm::vec3> velocities;

  template <typename Archive>
  void serialize(Archive &ar) {
    ar(facingDir, positions, rotations, velocities);
  }
};

struct MotionDatabaseFeature {
  // 12: left & right foot position, velocity
  //  6: hip position, velocity
  //  6: trajectory xz positions (3 future positions)
  std::array<float, 24> data;

  template <typename Archive>
  void serialize(Archive &ar) {
    ar(data);
  }
};

struct MotionDatabase {
  // nframes * ndata_dim
  std::vector<MotionDatabaseData> data;
  // nanim * 2 (begin, end)
  std::vector<std::pair<int, int>> range;
  // nframes * nfeat_dim
  std::vector<MotionDatabaseFeature> features;

  int dataFPS = 30;
  float trajInterval = 0.2f;

  // compute the features based on given joint index
  void ComputeFeatures(int lfoot, int rfoot, int hip);

  MotionDatabaseFeature CompressFeature(std::array<glm::vec3, 2> &hip,
                                        std::array<glm::vec3, 2> &lfoot,
                                        std::array<glm::vec3, 2> &rfoot,
                                        std::array<glm::vec2, 3> &traj);

  // search database for closest motion,
  // returns index for the closet motion frames
  int Query(MotionDatabaseFeature &feature);

  template <typename Archive>
  void serialize(Archive &ar) {
    ar(range, data,features, dataFPS, trajInterval);
  }
};

class MotionMatching : public Scriptable {
public:
  MotionMatching();
  ~MotionMatching();

  void OnEnable() override;
  void OnDisable() override;

  void Update(float dt) override;
  void LateUpdate(float dt) override;
  void DrawToScene() override;

private:
  void DrawInspectorGUI() override;

  std::string getInspectorWindowName() override { return "Motion Matching"; }

  // joystick related
  std::vector<int> availableJoysticks;
  std::vector<std::string> joystickNames;
  int _cjInd = 0, currentJoystick = -1;
  glm::vec2 leftInput = glm::vec2(0.0f);
  glm::vec2 rightInput = glm::vec2(0.0f);
  void queryJoysticks();

  // player related
  bool orbitCamera = false;
  float cameraOffset = 8.0f, cameraAngle = -40.0f;
  float cameraSensitivity = 3.0f;
  float playerSpeed = 3.0f, speedHalfLife = 1.0f;
  glm::vec3 speed = glm::vec3(0.0f);
  glm::vec3 cameraFacing = Entity::WorldForward;
  // this is the player position projected to xz plane
  glm::vec3 playerPosition = glm::vec3(0.0f);
  // this vector will always on xz plane
  glm::vec3 playerFacing = Entity::WorldForward;

  int trajCount = 4;
  float trajInterval = 0.2f;
  std::vector<glm::vec3> trajPos;
  std::vector<glm::vec3> trajSpeed;
};

}; // namespace aEngine