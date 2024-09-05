/**
 * Plot some metrics of current motion to Inspector window for easier comparing.
 */
#pragma once

#include "API.hpp"

#include "Function/Animation/Metrics.hpp"

namespace aEngine {

class VisMetrics : public Scriptable {
public:
  void Update(float dt) override {}

  void DrawInspectorGUI() override {
    drawInspectorGUIDefault();
    auto animator = entity->GetComponent<Animator>();
    ImGui::MenuItem("Slide Metrics", nullptr, nullptr, false);
    ImGui::SliderFloat("Height Threshold", &heightThreshold, 0.0f, 10.0f);
    ImGui::TextWrapped("Value: %.3f", animator == nullptr
                                          ? -1
                                          : slideMetricsCurrentPose(animator));
  }

private:
  float heightThreshold = 2.5f;
  std::vector<glm::vec3> prevPositions;

  std::string getTypeName() override { return "Visualize Metrics"; }

  float slideMetricsCurrentPose(std::shared_ptr<Animator> animator) {
    auto sm = animator->SkeletonMap;
    std::vector<glm::vec3> currentPositions;
    for (auto ele : sm) {
      currentPositions.push_back(ele.second.joint->Position());
    }
    if (prevPositions.size() == 0)
      prevPositions = currentPositions;
    return Animation::SlideMetrics(prevPositions, currentPositions,
                                   heightThreshold);
  }
};

}; // namespace aEngine