#include "System/Animation/AnimationSystem.hpp"
#include "Component/Animator.hpp"
#include "Component/Camera.hpp"
#include "Function/Animation/Deform.hpp"
#include "Function/Animation/Motion.hpp"
#include "Function/Render/Mesh.hpp"
#include "Function/Render/VisUtils.hpp"
#include "Scene.hpp"


namespace aEngine {

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
        // Animation::Pose CurrentPose = animator.motion->GetRestPose();
        int motionDataJointNum = animator->actor->GetNumJoints();
        // update the local positions of skeleton hierarchy
        // with animator's currentPose
        std::map<std::string, Entity *> skeletonHierarchy;
        BuildSkeletonHierarchy(animator->skeleton, skeletonHierarchy);
        if (skeletonHierarchy.size() != motionDataJointNum) {
          LOG_F(ERROR, "skeleton and motion data miss match for %s",
                GWORLD.EntityFromID(animator->GetID())->name.c_str());
          continue; // process the next animator data
        }
        // the skeleton hierarchy entities should have
        // the same name as in the motion data
        auto root =
            skeletonHierarchy.find(animator->motion->skeleton.jointNames[0]);
        if (root == skeletonHierarchy.end()) {
          LOG_F(ERROR, "root joint not found for %s", entity->name.c_str());
          continue;
        }
        // setup the root translation first
        root->second->SetLocalPosition(CurrentPose.rootLocalPosition);
        for (int boneInd = 0; boneInd < motionDataJointNum; ++boneInd) {
          std::string boneName = animator->actor->jointNames[boneInd];
          auto boneEntity = skeletonHierarchy.find(boneName);
          if (boneEntity == skeletonHierarchy.end()) {
            LOG_F(ERROR, "boneName %s not found in skeleton entities %s",
                  boneName.c_str(), animator->skeleton->name.c_str());
            break;
          }
          // setup local position and rotation for the bone entity
          // let hierarchy update system finish the rest
          boneEntity->second->SetLocalRotation(
              CurrentPose.jointRotations[boneInd]);
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