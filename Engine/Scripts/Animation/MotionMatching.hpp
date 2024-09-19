/**
 * Thanks to: https://theorangeduck.com/page/code-vs-data-driven-displacement
 *
 * This is a implementation to a character controller based on motion matching,
 * attach this script to some entity with `Animator` component, setup the
 * motion database to use. Then you should be free to control the character.
 */
#pragma once

#include "API.hpp"

namespace aEngine {

struct MotionDatabaseData {
  glm::vec3 facingDir = glm::vec3(0.0f);
  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> velocities;

  template <typename Archive> void serialize(Archive &archive) {
    archive(facingDir, positions, velocities);
  }
};

class MotionMatching : public Scriptable {
public:
  MotionMatching();
  ~MotionMatching();

  void Start() override;
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
  glm::vec3 playerPosition = glm::vec3(0.0f);
  glm::vec3 playerFacing = Entity::WorldForward;
};

}; // namespace aEngine