#include "Function/Animation/Motion.hpp"
#include "Function/Animation/Motion.hpp"
#include "Function/Math/Math.hpp"

namespace aEngine {

namespace Animation {

// Positional jitter (3rd derivative): smootheness of motion.
void JerkMetrics(Motion &gt, Motion &pred);

// Foot skating from Mode-Adaptive Neural Networks for Quadruped Motion Control.
void SlideMetrics(Motion &gt, Motion &pred);

};

};