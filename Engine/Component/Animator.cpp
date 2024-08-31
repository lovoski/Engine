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

void Animator::DrawSkeletonHierarchy() {
  int numActiveJoints = 0;
  for (int i = 0; i < actor->GetNumJoints(); ++i)
    numActiveJoints += jointActive[i];
  ImGui::MenuItem(
      ("Num Joints: " + std::to_string(numActiveJoints)).c_str(), nullptr,
      nullptr, false);
  for (int i = 0; i < actor->GetNumJoints(); ++i) {
    int depth = 1, cur = i;
    while (actor->jointParent[cur] != -1) {
      cur = actor->jointParent[cur];
      depth++;
    }
    std::string depthHeader = "";
    for (int j = 0; j < depth; ++j)
      depthHeader.push_back('-');
    depthHeader.push_back('|');
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
}

void Animator::DrawInspectorGUI() {
  if (ImGui::TreeNode("Animator")) {
    ImGui::MenuItem("Motion", nullptr, nullptr, false);
    ImGui::TextWrapped("FPS: %d", motion == nullptr ? -1 : motion->fps);
    ImGui::BeginChild("choosemotionsource", {-1, 30});
    static char motionSequencePath[100] = {0};
    sprintf(motionSequencePath, motionName.c_str());
    ImGui::InputTextWithHint("##motionsource", "Motion Sequence Path",
                             motionSequencePath, sizeof(motionSequencePath),
                             ImGuiInputTextFlags_ReadOnly);
    ImGui::SameLine();
    if (ImGui::Button("Clear", {-1, -1})) {
      if (skeleton && motion) {
        // reset skeleton to rest pose
        auto restPose = motion->GetRestPose();
        std::map<std::string, Entity *> hierarchyMap;
        BuildSkeletonHierarchy(skeleton, hierarchyMap);
        if (restPose.jointRotations.size() == hierarchyMap.size()) {
        }
      }
      motion = nullptr;
      motionName = "";
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
    ImGui::Checkbox("Skeleton On Top", &SkeletonOnTop);
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
    if (ImGui::Button("Clear", {-1, -1})) {
      skeleton = nullptr;
      skeletonName = "";
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
      DrawSkeletonHierarchy();
    }

    ImGui::TreePop();
  }
}

std::vector<glm::mat4> Animator::GetSkeletonTransforms() {
  // capture position, rotation of joints,
  // convert these information into matrices
  std::vector<glm::mat4> result(actor->GetNumJoints(), glm::mat4(1.0f));
  if (skeleton != nullptr && GWORLD.EntityValid(skeleton->ID)) {
    std::map<std::string, Entity *> hierarchyMap;
    BuildSkeletonHierarchy(skeleton, hierarchyMap);
    for (int jointInd = 0; jointInd < hierarchyMap.size(); ++jointInd) {
      auto jointName = actor->jointNames[jointInd];
      auto jointEntity = hierarchyMap[actor->jointNames[jointInd]];
      result[jointInd] =
          jointEntity->GetModelMatrix() * actor->offsetMatrices[jointInd];
    }
  }
  return result;
}

}; // namespace aEngine