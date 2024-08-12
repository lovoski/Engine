#pragma once

#include "Base/BaseSystem.hpp"

#include "Component/Animator.hpp"

namespace aEngine {

class AnimationSystem : public aEngine::BaseSystem {
public:
  AnimationSystem() {
    AddComponentSignature<Animator>();
  }
  ~AnimationSystem() {}

  void Update(float dt) override;

  // If the animator's ShowSkeleton is true,
  // draw its skeleton onto the scene.
  void Render();

private:
  float lastFrameTime;

};

};