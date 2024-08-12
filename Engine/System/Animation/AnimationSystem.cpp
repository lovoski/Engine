#include "System/Animation/AnimationSystem.hpp"
#include "Utils/Animation/Motion.hpp"
#include "Utils/Render/VisUtils.hpp"
#include "Component/Animator.hpp"
#include "Component/Camera.hpp"
#include "Scene.hpp"

namespace aEngine {

void AnimationSystem::Update(float dt) {
  for (auto id : entities) {
    auto entity = GWORLD.EntityFromID(id);
    auto animator = entity->GetComponent<Animator>();
    if (animator.skeleton != nullptr && animator.motion != nullptr) {
      int nFrames = animator.motion->poses.size();
      if (nFrames != 0) {
        animator.CurrentFrame =
            animator.CurrentFrame * (1.0f / animator.motion->fps) + dt;
        // make sure current frame is in range
        while (animator.CurrentFrame > nFrames)
          animator.CurrentFrame -= nFrames;
        animator.CurrentPose = animator.motion->At(animator.CurrentFrame);
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
        for (int boneInd = 0; boneInd < motionDataJointNum; ++boneInd) {
          std::string boneName =
              animator.CurrentPose.skeleton->jointNames[boneInd];
          auto boneEntity = skeletonHierarchy.find(boneName);
          if (boneEntity == skeletonHierarchy.end()) {
            Console.Log("[error]: boneName %s not found in skeleton entities %s\n",
                boneName.c_str(), animator.skeleton->name.c_str());
            break;
          }
          // setup local position and rotation for the bone entity
          // let hierarchy update system finish the rest
          boneEntity->second->localPosition = animator.CurrentPose.jointPositions[boneInd];
          boneEntity->second->localRotation = animator.CurrentPose.jointRotations[boneInd];
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
    glDisable(GL_DEPTH_TEST);
    for (auto id : entities) {
      auto entity = GWORLD.EntityFromID(id);
      auto animator = entity->GetComponent<Animator>();
      if (animator.ShowSkeleton && animator.skeleton != nullptr && animator.motion != nullptr) {
        // draw animator.CurrentPose
        auto positions = animator.CurrentPose.GetGlobalPositions();
        auto children = animator.CurrentPose.skeleton->jointChildren;
        auto skel = animator.CurrentPose.skeleton;
        for (int jointInd = 0; jointInd < skel->GetNumJoints(); ++jointInd) {
          for (auto cJoint : skel->jointChildren[jointInd]) {
            // draw line to all its children
            VisUtils::DrawLine3D(positions[jointInd], positions[cJoint], vp, glm::vec3(0.0f, 1.0f, 0.0f));
          }
        }
      }
    }
    glEnable(GL_DEPTH_TEST);
  }
}

}; // namespace aEngine