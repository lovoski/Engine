#include "Component/Animator.hpp"
#include "Component/DeformRenderer.hpp"
#include "Component/MeshRenderer.hpp"

#include "Function/Animation/Deform.hpp"
#include "Function/Animation/Edit.hpp"
#include "Function/AssetsLoader.hpp"
#include "Function/AssetsType.hpp"
#include "Function/GUI/Helpers.hpp"
#include "Function/Render/Mesh.hpp"
#include "Function/Render/RenderPass.hpp"
#include "Function/Render/Shader.hpp"
#include "Function/Render/VisUtils.hpp"

namespace aEngine {

Animator::Animator(EntityID id, Animation::Skeleton *act)
    : actor(act), BaseComponent(id) {
  createSkeletonEntities();
  BuildMappings();
}

Animator::Animator(EntityID id, Animation::Motion *m)
    : motion(m), BaseComponent(id) {
  actor = &motion->skeleton;
  createSkeletonEntities();
  BuildMappings();
}

Animator::~Animator() {}

void Animator::BuildMappings() {
  jointEntityMap.clear();
  jointEntityMap.resize(actor->GetNumJoints(), nullptr);
  jointActiveMap.clear();
  jointActiveMap.resize(actor->GetNumJoints(), true);
  if (skeleton == nullptr) {
    LOG_F(ERROR, "can't build jointEntityMap when root entity is nullptr");
    return;
  }
  std::stack<EntityID> s;
  s.push(skeleton->ID);
  int indexForAdditionalJoint = actor->jointNames.size();
  while (!s.empty()) {
    EntityID cur = s.top();
    auto curEnt = GWORLD.EntityFromID(cur).get();
    auto it1 = jointNameToInd.find(curEnt->name);
    if (it1 == jointNameToInd.end()) {
      // this is an additional joint
      jointNameToInd[curEnt->name] = indexForAdditionalJoint++;
      jointEntityMap.push_back(curEnt);
      jointActiveMap.push_back(true);
    } else {
      // this is an existing joint
      jointEntityMap[it1->second] = curEnt;
      jointActiveMap[it1->second] = true;
    }
    s.pop();
    for (auto c : curEnt->children)
      s.push(c->ID);
  }
}

void Animator::ApplyPoseToSkeleton(Animation::Pose &pose) {
  int motionJointNum = pose.skeleton->GetNumJoints();
  if (skeleton == nullptr) {
    LOG_F(WARNING,
          "actor has no skeleton entity root, can't apply motion to it");
    return;
  }
  auto root = jointNameToInd.find(pose.skeleton->jointNames[0]);
  if (root == jointNameToInd.end()) {
    LOG_F(ERROR, "root joint %s not found in skeleton, can't apply motion",
          pose.skeleton->jointNames[0].c_str());
    return;
  }
  int missingJointsFromEntity = 0;
  std::vector<std::string> missingJointsName;
  // apply root translation
  jointEntityMap[root->second]->SetLocalPosition(pose.rootLocalPosition);
  // apply joint rotations for joints defined in the pose
  for (int poseJointInd = 0; poseJointInd < motionJointNum; ++poseJointInd) {
    auto boneName = pose.skeleton->jointNames[poseJointInd];
    auto jointActorInd = jointNameToInd.find(boneName);
    if (jointActorInd == jointNameToInd.end()) {
      missingJointsFromEntity++;
      missingJointsName.push_back(boneName);
    } else {
      auto jointEntity = jointEntityMap[jointActorInd->second];
      jointEntity->SetLocalRotation(pose.jointRotations[poseJointInd]);
    }
  }
  // output the missing joints
  if (missingJointsFromEntity > 0) {
    std::string nameString = "";
    for (auto &name : missingJointsName)
      nameString = nameString + ", " + name;
    LOG_F(WARNING, "%d joints missing from entity skeleton, names: %s",
          missingJointsFromEntity, nameString.c_str());
  }
}

void Animator::createSkeletonEntities() {
  std::vector<Entity *> joints;
  jointEntityMap.resize(actor->GetNumJoints(), nullptr);
  jointActiveMap.resize(actor->GetNumJoints(), true);
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
    // cache mappings
    // 1. name to index
    jointNameToInd[actor->jointNames[i]] = i;
    // 2. index to entity
    jointEntityMap[i] = c.get();
    // 3. index to active
    jointActiveMap[i] = true;
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
  if (ImGui::Button("Export Actor Skeleton", {-1, 30}))
    actor->ExportAsBVH("exported_skeleton.bvh");
  int numActiveJoints = 0;
  for (int i = 0; i < actor->GetNumJoints(); ++i)
    numActiveJoints += jointActiveMap[i] ? 1 : 0;
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
    bool currentJointActiveStatus = jointActiveMap[i];
    if (ImGui::Checkbox(("##" + std::to_string(i)).c_str(),
                        &currentJointActiveStatus)) {
      if (!currentJointActiveStatus) {
        // disable all children at the disable of parent
        std::queue<int> q;
        q.push(i);
        while (!q.empty()) {
          auto tmpCur = q.front();
          jointActiveMap[tmpCur] = false;
          q.pop();
          for (auto c : actor->jointChildren[tmpCur])
            q.push(c);
        }
      } else
        jointActiveMap[i] = true;
    }
    ImGui::SameLine();
    ImGui::Text("%s %s", depthHeader.c_str(), actor->jointNames[i].c_str());
  }
  ImGui::EndChild();
}

void Animator::DrawInspectorGUI() {
  ImGui::MenuItem("Motion", nullptr, nullptr, false);
  ImGui::Checkbox("Show Trajectory", &ShowTrajectory);
  ImGui::SliderInt("Trajectory Count", &TrajCount, 1, 5);
  ImGui::SliderFloat("Trajectory Interval", &TrajInterval, 0.01f, 1.0f);
  ImGui::TextWrapped("FPS: %d", motion == nullptr ? -1 : motion->fps);
  ImGui::TextWrapped("Duration: %d",
                     motion == nullptr ? -1 : motion->poses.size());

  if (ImGui::Button("Export BVH Motion##animator", {-1, 30}))
    if (motion != nullptr)
      motion->SaveToBVH("./save_motion.bvh");
  static std::vector<char> motionPathBuffer(200);
  if (motion)
    std::strcpy(motionPathBuffer.data(), motion->path.c_str());
  GUIUtils::DragableFileTarget(
      "##motionsequencepath", "Motion Sequence Path",
      [&](std::string filename) {
        fs::path filepath(filename);
        auto extension = filepath.extension().string();
        if (extension == ".bvh" || extension == ".fbx") {
          motion = Loader.GetMotion(filename);
          if (motion != nullptr)
            return true;
          else
            return false;
        } else {
          LOG_F(WARNING, "animator only accepts .bvh and .fbx motion");
          return false;
        }
      },
      motionPathBuffer,
      [&]() {
        motion = nullptr;
        auto restPose = actor->GetRestPose();
        ApplyPoseToSkeleton(restPose);
      });
  if (ImGui::TreeNode("Motion Edit")) {
    if (motion == nullptr)
      ImGui::BeginDisabled();
    if (ImGui::Checkbox("Make Loop Motion", &makeLoopMotion)) {
      if (!makeLoopMotion) {
        motion = motionBackup;
      } else {
        motionBackup = motion;
        loopMotionBackup = std::make_unique<Animation::Motion>(
            Animation::MakeLoopMotion(*motion));
        motion = loopMotionBackup.get();
      }
    }
    if (motion == nullptr)
      ImGui::EndDisabled();
    ImGui::TreePop();
  }

  ImGui::Separator();
  ImGui::MenuItem("Skeleton", nullptr, nullptr, false);
  ImGui::Checkbox("Show Skeleton", &ShowSkeleton);
  ImGui::Checkbox("Show Joints", &ShowJoints);
  ImGui::SliderFloat("Joint Size", &JointVisualSize, 0.0f, 1.2f);
  ImGui::Checkbox("Helpers On Top", &SkeletonOnTop);
  ImGui::BeginChild("chooseskeletonroot", {-1, 30});
  if (skeleton != nullptr && GWORLD.EntityValid(skeleton->ID))
    skeletonName = skeleton->name;
  char skeletonNameBuf[100];
  sprintf(skeletonNameBuf, skeletonName.c_str());
  ImGui::InputTextWithHint("##skeletonentity", "Skeleton Root Entity",
                           skeletonNameBuf, sizeof(skeletonNameBuf),
                           ImGuiInputTextFlags_ReadOnly);
  ImGui::SameLine();
  if (ImGui::Button("Rest Pose", {-1, -1})) {
    auto restPose = actor->GetRestPose();
    ApplyPoseToSkeleton(restPose);
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
  GUIUtils::ColorEdit3("Color", SkeletonColor);
  if (ImGui::TreeNode("Hierarchy")) {
    drawSkeletonHierarchy();
    ImGui::TreePop();
  }
}

std::vector<BoneMatrixBlock> Animator::GetSkeletonTransforms() {
  // capture position, rotation of joints,
  // convert these information into matrices
  std::vector<BoneMatrixBlock> result(jointEntityMap.size());
  if (skeleton != nullptr && GWORLD.EntityValid(skeleton->ID)) {
    for (int i = 0; i < jointEntityMap.size(); ++i) {
      result[i].BoneModelMatrix = jointEntityMap[i]->GlobalTransformMatrix();
      result[i].BoneOffsetMatrix = actor->offsetMatrices[i];
    }
  }
  return result;
}

}; // namespace aEngine

REGISTER_COMPONENT(aEngine, Animator);