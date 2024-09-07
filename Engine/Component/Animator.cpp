#include "Component/Animator.hpp"
#include "Component/DeformRenderer.hpp"
#include "Component/MeshRenderer.hpp"

#include "Function/Animation/Deform.hpp"
#include "Function/AssetsLoader.hpp"
#include "Function/AssetsType.hpp"
#include "Function/Render/Mesh.hpp"
#include "Function/Render/RenderPass.hpp"
#include "Function/Render/Shader.hpp"
#include "Function/Render/VisUtils.hpp"

namespace aEngine {

void Animator::BuildSkeletonMap() {
  SkeletonMap.clear();
  if (skeleton == nullptr) {
    LOG_F(ERROR, "can't build SkeletonMap when root entity is nullptr");
    return;
  }
  std::stack<EntityID> s;
  s.push(skeleton->ID);
  while (!s.empty()) {
    EntityID cur = s.top();
    auto curEnt = GWORLD.EntityFromID(cur).get();
    SkeletonMapData smd;
    smd.joint = curEnt;
    smd.actorInd = findActorJointIndWithName(curEnt->name);
    if (smd.actorInd == -1)
      smd.active = false;
    else
      smd.active = jointActive[smd.actorInd] == 1;
    SkeletonMap.insert(std::make_pair(curEnt->name, smd));
    s.pop();
    for (auto c : curEnt->children)
      s.push(c->ID);
  }
}

void Animator::ApplyMotionToSkeleton(Animation::Pose &pose) {
  int motionJointNum = pose.skeleton->GetNumJoints();
  if (skeleton == nullptr) {
    LOG_F(WARNING,
          "actor has no skeleton entity root, can't apply motion to it");
    return;
  }
  if (SkeletonMap.size() != motionJointNum) {
    LOG_F(WARNING, "skeleton entity joint num and motion joint num mismatch, "
                 "motion maybe incorrect.");
  }
  auto root = SkeletonMap.find(pose.skeleton->jointNames[0]);
  if (root == SkeletonMap.end()) {
    LOG_F(ERROR, "root joint %s not found in skeleton, can't apply motion",
          pose.skeleton->jointNames[0].c_str());
    return;
  }
  root->second.joint->SetLocalPosition(pose.rootLocalPosition);
  for (int boneInd = 0; boneInd < motionJointNum; ++boneInd) {
    auto boneName = pose.skeleton->jointNames[boneInd];
    auto bone = SkeletonMap.find(boneName);
    if (bone == SkeletonMap.end()) {
      LOG_F(WARNING, "joint %s not found in skeleton, can't apply motion",
            boneName.c_str());
    } else {
      bone->second.joint->SetLocalRotation(pose.jointRotations[boneInd]);
    }
  }
}

void Animator::createSkeletonEntities() {
  std::vector<Entity *> joints;
  for (int i = 0; i < actor->GetNumJoints(); ++i) {
    auto c = GWORLD.AddNewEntity();
    c->name = actor->jointNames[i];
    c->SetLocalPosition(actor->jointOffset[i]);
    c->SetLocalRotation(actor->jointRotation[i]);
    c->SetLocalScale(actor->jointScale[i]);
    joints.push_back(c.get());
    if (i == 0)
      joints[i]->parent = nullptr;
    else {
      joints[i]->parent = joints[actor->jointParent[i]];
      joints[i]->parent->children.push_back(joints[i]);
    }
  }
  if (joints.size() >= 1)
    skeleton = joints[0];
  else
    LOG_F(WARNING, "actor has no joints, don't create skeleton hierarchy");
  // set the parent of skeleton to entity holding this animator
  auto entityInstance = GWORLD.EntityFromID(entityID);
  entityInstance->children.push_back(skeleton);
  skeleton->parent = entityInstance.get();
}

void Animator::drawSkeletonHierarchy() {
  if (ImGui::Button("Export BVH Skeleton", {-1, 30})) {
    actor->ExportAsBVH("export_skeleton.bvh");
  }
  int numActiveJoints = 0;
  for (int i = 0; i < actor->GetNumJoints(); ++i)
    numActiveJoints += jointActive[i];
  ImGui::MenuItem(("Num Joints: " + std::to_string(numActiveJoints)).c_str(),
                  nullptr, nullptr, false);
  ImGui::BeginChild("skeletonhierarchy", {-1, -1});
  for (int i = 0; i < actor->GetNumJoints(); ++i) {
    int depth = 1, cur = i;
    while (actor->jointParent[cur] != -1) {
      cur = actor->jointParent[cur];
      depth++;
    }
    std::string depthHeader = "";
    for (int j = 0; j < depth; ++j)
      depthHeader.push_back('-');
    depthHeader.push_back(':');
    bool active = jointActive[i];
    if (ImGui::Checkbox(("##" + std::to_string(i)).c_str(), &active)) {
      jointActive[i] = (int)active;
      if (!active) {
        // disable all children at the disable of parent
        std::queue<int> q;
        q.push(i);
        while (!q.empty()) {
          auto cur = q.front();
          q.pop();
          for (auto c : actor->jointChildren[cur]) {
            jointActive[c] = 0;
            q.push(c);
          }
        }
      }
    }
    ImGui::SameLine();
    ImGui::Text("%s %s", depthHeader.c_str(), actor->jointNames[i].c_str());
  }
  ImGui::EndChild();
}

void Animator::DrawInspectorGUI() {
  if (ImGui::TreeNode("Animator")) {
    ImGui::MenuItem("Motion", nullptr, nullptr, false);
    ImGui::TextWrapped("FPS: %d", motion == nullptr ? -1 : motion->fps);
    ImGui::TextWrapped("Duration: %d",
                       motion == nullptr ? -1 : motion->poses.size());
    if (ImGui::Button("Export BVH Motion", {-1, 30})) {
      if (motion != nullptr)
        motion->SaveToBVH("./save_motion.bvh");
    }
    ImGui::BeginChild("choosemotionsource", {-1, 30});
    static char motionSequencePath[200] = {0};
    sprintf(motionSequencePath, motionName.c_str());
    ImGui::InputTextWithHint("##motionsource", "Motion Sequence Path",
                             motionSequencePath, sizeof(motionSequencePath),
                             ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if (ImGui::Button("Clear", {-1, -1})) {
      // reset skeleton to rest pose
      ApplyMotionToSkeleton(actor->GetRestPose());
      // clear variables
      motion = nullptr;
      motionName = "";
      for (int i = 0; i < sizeof(motionSequencePath); ++i)
        motionSequencePath[i] = 0;
    }
    ImGui::EndChild();
    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload =
              ImGui::AcceptDragDropPayload("ASSET_FILENAME")) {
        char *assetFilename = (char *)payload->Data;
        fs::path filepath = fs::path(assetFilename);
        std::string extension = filepath.extension().string();
        if (extension == ".bvh" || extension == ".fbx") {
          motion = Loader.GetMotion(filepath.string());
          motionName = filepath.string();
        } else {
          LOG_F(ERROR, "only .bvh and .fbx motion are supported");
        }
      }
      ImGui::EndDragDropTarget();
    }
    ImGui::Separator();
    ImGui::MenuItem("Skeleton", nullptr, nullptr, false);
    ImGui::Checkbox("Show Skeleton", &ShowSkeleton);
    ImGui::Checkbox("Show Joints", &ShowJoints);
    ImGui::SliderFloat("Joint Size", &JointVisualSize, 0.0f, 1.2f);
    ImGui::Checkbox("Helpers On Top", &SkeletonOnTop);
    ImGui::BeginChild("chooseskeletonroot", {-1, 30});
    if (skeleton != nullptr && GWORLD.EntityValid(skeleton->ID)) {
      skeletonName = skeleton->name;
    }
    char skeletonNameBuf[100];
    sprintf(skeletonNameBuf, skeletonName.c_str());
    ImGui::InputTextWithHint("##skeletonentity", "Skeleton Root Entity",
                             skeletonNameBuf, sizeof(skeletonNameBuf),
                             ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if (ImGui::Button("Rest Pose", {-1, -1})) {
      ApplyMotionToSkeleton(actor->GetRestPose());
    }
    ImGui::EndChild();
    if (ImGui::BeginDragDropTarget()) {
      if (const ImGuiPayload *payload =
              ImGui::AcceptDragDropPayload("ENTITYID_DATA")) {
        Entity *skeletonRoot = *(Entity **)payload->Data;
        skeleton = skeletonRoot;
      }
      ImGui::EndDragDropTarget();
    }
    float skeletonColor[3] = {SkeletonColor.x, SkeletonColor.y,
                              SkeletonColor.z};
    if (ImGui::ColorEdit3("Color", skeletonColor)) {
      SkeletonColor =
          glm::vec3(skeletonColor[0], skeletonColor[1], skeletonColor[2]);
    }
    if (ImGui::CollapsingHeader("Hierarchy")) {
      drawSkeletonHierarchy();
    }

    ImGui::TreePop();
  }
}

std::vector<glm::mat4> Animator::GetSkeletonTransforms() {
  // capture position, rotation of joints,
  // convert these information into matrices
  std::vector<glm::mat4> result(actor->GetNumJoints(), glm::mat4(1.0f));
  if (skeleton != nullptr && GWORLD.EntityValid(skeleton->ID)) {
    for (int jointInd = 0; jointInd < SkeletonMap.size(); ++jointInd) {
      auto jointName = actor->jointNames[jointInd];
      auto skelData = SkeletonMap[actor->jointNames[jointInd]];
      result[jointInd] =
          skelData.joint->GetModelMatrix() * actor->offsetMatrices[jointInd];
    }
  }
  return result;
}

int Animator::findActorJointIndWithName(std::string name) {
  int jointNum = actor->GetNumJoints();
  for (int i = 0; i < jointNum; ++i) {
    auto jointName = actor->jointNames[i];
    if (jointName == name)
      return i;
  }
  return -1;
}

}; // namespace aEngine