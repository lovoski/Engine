/**
 * Plot some metrics of current motion to Inspector window for easier comparing.
 */
#pragma once

#include "API.hpp"

#include "Function/Animation/Metrics.hpp"

namespace aEngine {

class VisMetrics : public Scriptable {
public:
  void Update(float dt) override;

  void drawCustomInspectorGUI() override;

  void DrawToScene() override;

private:
  float heightThreshold = 2.5f;
  bool showContactJoint = false;
  std::vector<glm::vec3> prevPositions;
  std::vector<Entity *> contactJoints;
  Entity *lFoot = nullptr, *rFoot = nullptr;
  Entity *meshBaseForIntersection = nullptr;

  std::string getTypeName() override { return "Visualize Metrics"; }

  void locateHumanoidFoot(std::shared_ptr<Animator> animator);

  float slideMetricsCurrentPose(std::shared_ptr<Animator> animator);
};

}; // namespace aEngine