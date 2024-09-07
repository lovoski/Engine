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
    ImGui::TextWrapped("Value: %.3f", animator == nullptr
                                          ? -1
                                          : slideMetricsCurrentPose(animator));
    ImGui::Checkbox("Show Contact", &showContactJoint);
    ImGui::SliderFloat("Height Threshold", &heightThreshold, 0.0f, 10.0f);
  }

  void DrawToScene() override {
    EntityID camera;
    if (GWORLD.GetActiveCamera(camera)) {
      glDisable(GL_DEPTH_TEST);
      auto cameraComp = GWORLD.GetComponent<Camera>(camera);
      if (showContactJoint) {
        auto animator = entity->GetComponent<Animator>();
        if (animator) {
          for (auto ele : animator->SkeletonMap) {
            auto pos = ele.second.joint->Position();
            if (pos.y < heightThreshold) {
              VisUtils::DrawWireSphere(pos, cameraComp->VP, 1.0f,
                                       glm::vec3(1.0f, 0.0f, 0.0f));
            }
          }
        }
      glEnable(GL_DEPTH_TEST);
      }
    }
  }

private:
  float heightThreshold = 2.5f;
  bool showContactJoint = false;
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