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
    std::swap(SystemStartFrame, SystemEndFrame);
  }
  // loop systemCurrentFrame in range
  int duration = SystemEndFrame - SystemStartFrame;
  if (duration != 0) {
    // avoid dead loop
    while (SystemCurrentFrame < SystemStartFrame)
      SystemCurrentFrame += duration;
    while (SystemCurrentFrame > SystemEndFrame)
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
        Animation::Pose CurrentPose = animator->motion->At(SystemCurrentFrame);
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

        // construct the drawQueue with only active joints
        std::vector<std::pair<glm::vec3, glm::vec3>> drawQueue;
        auto actor = animator->actor;
        int numJoints = actor->GetNumJoints();
        std::vector<int> visitsRemains(numJoints, 0), startPoints;
        for (int i = 0; i < numJoints; ++i) {
          visitsRemains[i] = actor->jointChildren.size();
          if (actor->jointChildren[i].size() == 0)
            startPoints.push_back(i);
        }
        for (auto startPointInd : startPoints) {
          int cur = startPointInd, parent;
          std::string curName = actor->jointNames[cur], parentName;
          SkeletonMapData curData, parentData;
          curData = animator->SkeletonMap[curName];
          int toBeMatched = -1;
          while (actor->jointParent[cur] != -1) {
            parent = actor->jointParent[cur];
            parentName = actor->jointNames[parent];
            parentData = animator->SkeletonMap[parentName];
            if (visitsRemains[parent] > 0) {
              if (curData.active && parentData.active) {
                drawQueue.push_back(std::make_pair(parentData.joint->Position(),
                                                   curData.joint->Position()));
                visitsRemains[parent]--;
              } else if (curData.active && !parentData.active) {
                toBeMatched = cur;
              } else if (!curData.active && parentData.active &&
                         toBeMatched != -1) {
                auto childData =
                    animator->SkeletonMap[actor->jointNames[toBeMatched]];
                drawQueue.push_back(std::make_pair(
                    parentData.joint->Position(), childData.joint->Position()));
                visitsRemains[parent]--;
              }
            } else
              break;
            cur = parent;
            curData = parentData;
            curName = parentName;
          }
        }

        if (animator->SkeletonOnTop)
          glDisable(GL_DEPTH_TEST);
        else
          glEnable(GL_DEPTH_TEST);
        // Draw the joint positions
        if (animator->motion) {
          Animation::Pose CurrentPose =
              animator->motion->At(SystemCurrentFrame);
          auto globalPos = CurrentPose.GetGlobalPositions();
          for (auto gPos : globalPos)
            VisUtils::DrawWireSphere(gPos, vp, 1.0f,
                                     glm::vec3(1.0f, 0.0f, 0.0f));
        }
        for (auto &drawPair : drawQueue) {
          VisUtils::DrawBone(drawPair.first, drawPair.second,
                             GWORLD.Context.sceneWindowSize, vp,
                             animator->SkeletonColor);
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