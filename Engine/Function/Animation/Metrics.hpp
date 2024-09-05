#include "Function/Animation/Motion.hpp"
#include "Function/Math/Math.hpp"

#include "Global.hpp"

namespace aEngine {

namespace Animation {

// Positional jitter (3rd derivative): smootheness of motion.
// Reference:
// https://github.com/Xinyu-Yi/TransPose/blob/main/articulate/evaluator.py#L269
void JerkMetrics(Motion &gt, Motion &pred);

// Foot sliding from Mode-Adaptive Neural Networks for Quadruped Motion Control.
// Reference: https://homepages.inf.ed.ac.uk/tkomura/dog.pdf
// When the height of a joint is lower than the `heightThreshold`, the velocity
// of which will be weighted by a factor (2-2^(h/H)) and added into the metric.
// h -> height of the joint, H -> height threshold
float SlideMetrics(std::vector<glm::vec3> &prev,
                   std::vector<glm::vec3> &current,
                   float heightThreshold);

// Average slide metrics for the whole motion.
float SlideMetrics(Motion &motion, float heightThreshold);

}; // namespace Animation

}; // namespace aEngine