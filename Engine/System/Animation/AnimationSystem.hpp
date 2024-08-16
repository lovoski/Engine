#pragma once

#include "Base/BaseSystem.hpp"

#include "Component/Animator.hpp"

namespace aEngine {

class AnimationSystem : public aEngine::BaseSystem {
public:
  AnimationSystem() {
    AddComponentSignatureRequireAll<Animator>();
    systemCurrentFrame = 0;
    systemStartFrame = 0;
    systemEndFrame = 1000;
  }
  ~AnimationSystem() {}

  void Update(float dt) override;

  // If the animator's ShowSkeleton is true,
  // draw its skeleton onto the scene.
  void Render();

  int systemFPS = 30;
  float systemCurrentFrame;
  int systemStartFrame, systemEndFrame;
  // automatically increase systemCurrentFrame according to dt and systemFPS
  bool enableAutoPlay = false;
};

};