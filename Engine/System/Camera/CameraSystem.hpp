#pragma once

#include "Global.hpp"
#include "Base/BaseSystem.hpp"

namespace aEngine {

class CameraSystem : public BaseSystem {
public:

  void Update() override;

private:
  bool mouseFirstMove = true;
  glm::vec2 mouseLastPos;
};

};