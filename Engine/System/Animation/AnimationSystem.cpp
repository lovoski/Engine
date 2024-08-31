#include "System/Animation/AnimationSystem.hpp"
#include "Component/Animator.hpp"
#include "Component/Camera.hpp"
#include "Function/Animation/Deform.hpp"
#include "Function/Animation/Motion.hpp"
#include "Function/Render/Mesh.hpp"
#include "Function/Render/VisUtils.hpp"
#include "Scene.hpp"


namespace aEngine {

void AnimationSystem::PreUpdate(float dt) {
  for (auto id : entities) {
    auto entity = GWORLD.EntityFromID(id);
    auto animator = entity->GetComponent<Animator>();
    animator->BuildSkeletonMap();
  }
}

void AnimationSystem::Update(float dt) {
  if (EnableAutoPlay) {
    // update the global system frame index
    SystemCurrentFrame += dt * SystemFPS;
  }
  if (SystemEndFrame - SystemStartFrame < 0) {
    // flip the start frame and end frame
    // if endframe < startframe
    std::swap(SystemStartFrame,
              SystemEndFrame);
  }
  // loop systemCurrentFrame in range
  int duration =
      SystemEndFrame - SystemStartFrame;
  if (duration != 0) {
    // avoid dead loop
    while (SystemCurrentFrame <
           SystemStartFrame)
      SystemCurrentFrame += duration;
    while (SystemCurrentFrame >
           SystemEndFrame)
      SystemCurrentFrame -= duration;
  }
  for (auto id : entities) {
    auto entity = GWORLD.EntityFromID(id);
    auto animator = entity->GetComponent<Animator>();
    // TODO: checking each entity in use is valid or not is redundent
    // We need smart pointer to avoid invalid pointers
    if (animator->skeleton != nullptr && animator->motion != nullptr &&
        GWORLD.EntityValid(animator->skeleton->ID)) {
      int nFrames = animator->motion->poses.size();
      if (nFrames != 0) {
        // sample animation from motion data of each animator
        Animation::Pose CurrentPose =
            animator->motion->At(SystemCurrentFrame);
        animator->ApplyMotionToSkeleton(CurrentPose);
      }
    }
  }
}

void AnimationSystem::Render() {
  EntityID camera;
  if (GWORLD.GetActiveCamera(camera)) {
    auto cameraObject = GWORLD.EntityFromID(camera);
    auto cameraComp = cameraObject->GetComponent<Camera>();
    auto vp = cameraComp->VP;
    // render skeleton if the flag is set
    for (auto id : entities) {
      auto entity = GWORLD.EntityFromID(id);
      auto animator = entity->GetComponent<Animator>();
      if (animator->ShowSkeleton && animator->skeleton != nullptr &&
          GWORLD.EntityValid(animator->skeleton->ID)) {
        // draw animator.skeleton
        auto root = animator->skeleton;
        std::queue<Entity *> q;
        q.push(root);
        if (animator->SkeletonOnTop)
          glDisable(GL_DEPTH_TEST);
        else
          glEnable(GL_DEPTH_TEST);
        while (!q.empty()) {
          auto cur = q.front();
          q.pop();
          for (auto c : cur->children) {
            q.push(c);
            VisUtils::DrawBone(cur->Position(), c->Position(),
                               GWORLD.Context.sceneWindowSize, vp,
                               animator->SkeletonColor);
          }
        }
        if (animator->SkeletonOnTop)
          glEnable(GL_DEPTH_TEST);
        else
          glDisable(GL_DEPTH_TEST);
      }
    }
  }
}

}; // namespace aEngine