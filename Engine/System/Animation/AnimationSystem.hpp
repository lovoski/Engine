#pragma once

#include "Base/BaseSystem.hpp"

#include "Component/Animator.hpp"

namespace aEngine {

class AnimationSystem : public aEngine::BaseSystem {
public:
  AnimationSystem() {
    Reset(); // initialize local variables
    AddComponentSignatureRequireAll<Animator>();
  }
  ~AnimationSystem() {}

  void Update(float dt) override;

  // If the animator's ShowSkeleton is true,
  // draw its skeleton onto the scene.
  void Render();

  void Reset() override {
    SystemFPS = 30;
    EnableAutoPlay = false;
    SystemStartFrame = 0;
    SystemEndFrame = 1000;
    SystemCurrentFrame = 0.0f;
  }

  int SystemFPS = 30;
  float SystemCurrentFrame;
  int SystemStartFrame, SystemEndFrame;
  // automatically increase systemCurrentFrame according to dt and systemFPS
  bool EnableAutoPlay = false;

};

};