#pragma once

#include "API.hpp"

namespace aEngine {

struct SAMERetarget : public Scriptable {

  std::string motionName = "";
  Animation::Motion *motion;

  void LateUpdate(float dt) override;

  void DrawInspectorGUI() override;

  std::string getTypeName() override { return "SAME Retarget"; }
};

}; // namespace aEngine