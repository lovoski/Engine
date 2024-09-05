#include "Function/Animation/Metrics.hpp"

namespace aEngine {

namespace Animation {

using glm::length;
using glm::vec3;
using std::pow;
using std::vector;

void JerkMetrics(Motion &gt, Motion &pred) {}

float SlideMetrics(vector<vec3> &prev, vector<vec3> &current,
                   float heightThreshold) {
  int numJoint = prev.size();
  if (std::abs(heightThreshold) < 1e-8f)
    heightThreshold += 1e-5f;
  if (prev.size() != current.size()) {
    LOG_F(WARNING, "prev global positions and current global positions "
                   "mismatch in dimension");
    numJoint = std::min(prev.size(), current.size());
  }
  vector<vec3> currentVelocity(numJoint, vec3(0.0f));
  for (int i = 0; i < numJoint; ++i)
    currentVelocity[i] = current[i] - prev[i];
  float metricValue = 0.0f;
  for (int i = 0; i < numJoint; ++i) {
    if (current[i].y <= heightThreshold) {
      metricValue += length(currentVelocity[i]) *
                     (2.0f - pow(2.0f, current[i].y / heightThreshold));
    }
  }
  return metricValue;
}

float SlideMetrics(Motion &motion, float heightThreshold) {
  float metricValue = 0.0f;
  auto prevPositions = motion.poses[0].GetGlobalPositions();
  auto currentPositions = prevPositions;
  for (int i = 1; i < motion.poses.size(); ++i) {
    currentPositions = motion.poses[i].GetGlobalPositions();
    metricValue +=
        SlideMetrics(prevPositions, currentPositions, heightThreshold);
    prevPositions = currentPositions;
  }
  return metricValue / (motion.poses.size());
}

}; // namespace Animation

}; // namespace aEngine