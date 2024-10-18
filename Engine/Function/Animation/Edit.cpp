#include "Function/Animation/Edit.hpp"

namespace aEngine::Animation {

Motion MakeLoopMotion(Motion &motion) {
  auto newMotion = motion;
  newMotion.path = "::loop_motion_" + motion.path;

  return newMotion;
}

}; // namespace aEngine::Animation