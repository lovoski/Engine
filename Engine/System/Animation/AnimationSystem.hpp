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

  void PreUpdate(float dt) override;
  void Update(float dt) override;

  // If the animator's ShowSkeleton is true,
  // draw its skeleton onto the scene.
  void DebugRender() override;

  void Reset() override {
    SystemFPS = 30;
    EnableAutoPlay = false;
    ShowSequencer = false;
    SystemStartFrame = 0;
    SystemEndFrame = 1000;
    SystemCurrentFrame = 0.0f;
  }

  int SystemFPS = 30;
  float SystemCurrentFrame;
  int SystemStartFrame, SystemEndFrame;
  // automatically increase systemCurrentFrame according to dt and systemFPS
  bool EnableAutoPlay = false;
  bool ShowSequencer = false;

  void DrawSequencer();

  template <typename Archive> void serialize(Archive &ar) {
    ar(cereal::base_class<BaseSystem>(this));
    ar(SystemFPS, SystemCurrentFrame, SystemStartFrame, SystemEndFrame,
       EnableAutoPlay, ShowSequencer);
  }

private:
  void collectSkeletonDrawQueue(
      std::shared_ptr<Animator> animator,
      std::vector<std::pair<glm::vec3, glm::vec3>> &drawQueue);
};

}; // namespace aEngine