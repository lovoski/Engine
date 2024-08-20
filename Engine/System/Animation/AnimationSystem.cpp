#include "System/Animation/AnimationSystem.hpp"
#include "Component/Animator.hpp"
#include "Component/Camera.hpp"
#include "Scene.hpp"
#include "Function/Animation/Motion.hpp"
#include "Function/Render/VisUtils.hpp"

namespace aEngine {

void AnimationSystem::Update(float dt) {
  if (enableAutoPlay) {
    // update the global system frame index
    systemCurrentFrame = systemCurrentFrame + dt * systemFPS;
  }
  if (systemEndFrame-systemStartFrame < 0) {
    // flip the start frame and end frame
    // if endframe < startframe
    std::swap(systemStartFrame, systemEndFrame);
  }
  // loop systemCurrentFrame in range
  int duration = systemEndFrame-systemStartFrame;
  if (duration != 0) {
    // avoid dead loop
    while (systemCurrentFrame < systemStartFrame)
      systemCurrentFrame += duration;
    while (systemCurrentFrame > systemEndFrame)
      systemCurrentFrame -= duration;
  }
  // Console.Log("currentframe=%.3f\n", systemCurrentFrame);
  for (auto id : entities) {
    auto entity = GWORLD.EntityFromID(id);
    auto &animator = entity->GetComponent<Animator>();
    if (animator.skeleton != nullptr && animator.motion != nullptr) {
      int nFrames = animator.motion->poses.size();
      if (nFrames != 0) {
        // sample animation from motion data of each animator
        animator.CurrentPose = animator.motion->At(systemCurrentFrame);
        int motionDataJointNum = animator.CurrentPose.skeleton->GetNumJoints();
        // update the local positions of skeleton hierarchy
        // with animator's currentPose
        std::map<std::string, Entity *> skeletonHierarchy;
        std::queue<EntityID> q;
        auto skel = animator.skeleton;
        q.push(skel->ID);
        while (!q.empty()) {
          EntityID cur = q.front();
          Entity *curEnt = GWORLD.EntityFromID(cur);
          skeletonHierarchy.insert(std::make_pair(curEnt->name, curEnt));
          q.pop();
          for (auto c : curEnt->children)
            q.push(c->ID);
        }
        if (skeletonHierarchy.size() != motionDataJointNum) {
          Console.Log("[error]: skeleton and motion data miss match for %s\n",
                      GWORLD.EntityFromID(animator.GetID())->name.c_str());
          continue; // process the next animator data
        }
        // the skeleton hierarchy entities should have
        // the same name as in the motion data
        auto root =
            skeletonHierarchy.find(animator.motion->skeleton.jointNames[0]);
        if (root == skeletonHierarchy.end()) {
          Console.Log("[error]: root joint not found for %s\n",
                      entity->name.c_str());
          continue;
        }
        // setup the root translation first
        root->second->localPosition = animator.CurrentPose.rootLocalPosition;
        for (int boneInd = 0; boneInd < motionDataJointNum; ++boneInd) {
          std::string boneName =
              animator.CurrentPose.skeleton->jointNames[boneInd];
          auto boneEntity = skeletonHierarchy.find(boneName);
          if (boneEntity == skeletonHierarchy.end()) {
            Console.Log(
                "[error]: boneName %s not found in skeleton entities %s\n",
                boneName.c_str(), animator.skeleton->name.c_str());
            break;
          }
          // setup local position and rotation for the bone entity
          // let hierarchy update system finish the rest
          boneEntity->second->localRotation =
              animator.CurrentPose.jointRotations[boneInd];
        }
      }
    }
  }
}

void AnimationSystem::Render() {
  EntityID camera;
  if (GWORLD.GetActiveCamera(camera)) {
    auto cameraObject = GWORLD.EntityFromID(camera);
    auto cameraComp = cameraObject->GetComponent<Camera>();
    auto viewport = GWORLD.Context.sceneWindowSize;
    auto viewMat = cameraComp.GetViewMatrix(*cameraObject);
    auto projMat = cameraComp.GetProjMatrixPerspective(viewport.x, viewport.y);
    auto vp = projMat * viewMat;
    // render skeleton if the flag is set
    for (auto id : entities) {
      auto entity = GWORLD.EntityFromID(id);
      auto &animator = entity->GetComponent<Animator>();
      if (animator.ShowSkeleton && animator.skeleton != nullptr &&
          animator.motion != nullptr) {
        // draw animator.CurrentPose
        auto root = animator.skeleton;
        std::queue<Entity *> q;
        q.push(root);
        if (animator.SkeletonOnTop)
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
                               animator.SkeletonColor);
          }
        }
        if (animator.SkeletonOnTop)
          glEnable(GL_DEPTH_TEST);
        else
          glDisable(GL_DEPTH_TEST);
      }
    }
  }
}

}; // namespace aEngine