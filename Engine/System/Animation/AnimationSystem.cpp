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
  // for (auto id : entities) {
  //   auto entity = GWORLD.EntityFromID(id);
  //   auto animator = entity->GetComponent<Animator>();
  //   animator->BuildSkeletonMap();
  // }
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

void AnimationSystem::collectSkeletonDrawQueue(
    std::shared_ptr<Animator> animator,
    std::vector<std::pair<glm::vec3, glm::vec3>> &drawQueue) {
  auto actor = animator->actor;
  int numJoints = actor->GetNumJoints();
  std::vector<int> visitsRemains(numJoints, 0), startPoints;
  // collect end effectors as start points
  for (int i = 0; i < numJoints; ++i) {
    visitsRemains[i] = actor->jointChildren[i].size();
    if (actor->jointChildren[i].size() == 0)
      startPoints.push_back(i);
  }
  // traverse from start points to root joint
  for (auto startPointInd : startPoints) {
    int cur = startPointInd, parent;
    std::string curName = actor->jointNames[cur], parentName;
    SkeletonMapData curData, parentData;
    curData = animator->SkeletonMap[animator->HashString(curName)];
    int toBeMatched = -1;
    while (actor->jointParent[cur] != -1) {
      parent = actor->jointParent[cur];
      parentName = actor->jointNames[parent];
      parentData = animator->SkeletonMap[animator->HashString(parentName)];
      if (visitsRemains[parent] > 0) {
        if (curData.active && parentData.active) {
          drawQueue.push_back(std::make_pair(parentData.joint->Position(),
                                             curData.joint->Position()));
          visitsRemains[parent]--;
        } else if (curData.active && !parentData.active) {
          toBeMatched = cur;
        } else if (!curData.active && parentData.active && toBeMatched != -1) {
          auto childData = animator->SkeletonMap[animator->HashString(
              actor->jointNames[toBeMatched])];
          drawQueue.push_back(std::make_pair(parentData.joint->Position(),
                                             childData.joint->Position()));
          visitsRemains[parent]--;
        }
      } else
        break;
      cur = parent;
      curData = parentData;
      curName = parentName;
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
      if ((animator->ShowSkeleton || animator->ShowJoints) &&
          animator->skeleton != nullptr &&
          GWORLD.EntityValid(animator->skeleton->ID)) {

        // construct the drawQueue with only active joints
        std::vector<std::pair<glm::vec3, glm::vec3>> drawQueue;
        if (animator->ShowSkeleton)
          collectSkeletonDrawQueue(animator, drawQueue);

        if (animator->SkeletonOnTop)
          glDisable(GL_DEPTH_TEST);
        else
          glEnable(GL_DEPTH_TEST);

        // if (animator->motion) {
        //   glm::vec3 rootProjPos = glm::vec3(animator->skeleton->Position());
        //   rootProjPos.y = 0.5f;
        //   glm::vec3 targetProjPos =
        //       20.0f *
        //       animator->motion->At(SystemCurrentFrame).GetFacingDirection();
        //   VisUtils::DrawArrow(rootProjPos, rootProjPos + targetProjPos, vp,
        //                       glm::vec3(1.0f, 0.0f, 0.0f),
        //                       glm::length(targetProjPos) * 0.2f);
        // }

        // Draw the bones
        VisUtils::DrawBones(drawQueue, GWORLD.Context.sceneWindowSize, vp,
                            animator->SkeletonColor);
        // Draw the joint positions
        if (animator->ShowJoints) {
          for (auto ele : animator->SkeletonMap) {
            VisUtils::DrawWireSphere(ele.second.joint->Position(), vp,
                                     animator->JointVisualSize,
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

void AnimationSystem::DrawSequencer() {
  ImGui::Begin("Timeline", &ShowSequencer);
  ImGui::BeginChild("TimelineProperties", {-1, -1});
  ImGui::PushItemWidth(-1);

  // animation take selection menu
  std::vector<std::string> takeNames{"Default", "Take 001", "Take 002"};
  static int currentActiveTake = 0;
  ImGui::Text("Current Take");
  ImGui::SameLine();
  if (ImGui::BeginCombo("##currenttakecombo",
                        takeNames[currentActiveTake].c_str())) {
    for (int comboIndex = 0; comboIndex < takeNames.size(); ++comboIndex) {
      bool isSelected = currentActiveTake == comboIndex;
      if (ImGui::Selectable(takeNames[comboIndex].c_str(), isSelected)) {
        currentActiveTake = comboIndex;
        // switch the active take
      }
      if (isSelected)
        ImGui::SetItemDefaultFocus();
    }
    ImGui::EndCombo();
  }
  ImGui::Separator();

  // play back properties
  ImGui::Checkbox("Auto Play", &EnableAutoPlay);
  ImGui::SameLine();
  int se[2] = {SystemStartFrame, SystemEndFrame};
  if (ImGui::InputInt2("##Start & End", se)) {
    SystemStartFrame = se[0];
    SystemEndFrame = se[1];
  }

  // current frame slider
  ImGui::SliderFloat("##currenrFrame", &SystemCurrentFrame, SystemStartFrame,
                     SystemEndFrame);
  ImGui::PopItemWidth();
  ImGui::EndChild();
  ImGui::End();
}

}; // namespace aEngine