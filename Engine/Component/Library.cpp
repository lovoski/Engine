#include "System/Animation/Common.hpp"

#include "Component/Animator.hpp"
#include "Component/MeshRenderer.hpp"
#include "Function/AssetsLoader.hpp"
#include "Function/AssetsType.hpp"
#include "Function/Render/MaterialData.hpp"
#include "Function/Render/Mesh.hpp"
#include "Function/Render/Shader.hpp"
#include "Function/Render/VisUtils.hpp"

namespace aEngine {

void Animator::DrawInspectorGUI() {
  if (ImGui::TreeNode("Animator")) {
    ImGui::MenuItem("Skeleton", nullptr, nullptr, false);
    ImGui::Checkbox("Show Skeleton", &ShowSkeleton);
    ImGui::Checkbox("Skeleton On Top", &SkeletonOnTop);
    ImGui::BeginChild("chooseskeletonroot", {-1, 30});
    if (skeleton != nullptr) {
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
          Console.Log("[error]: only .bvh and .fbx motion are supported\n");
        }
      }
      ImGui::EndDragDropTarget();
    }
    ImGui::TreePop();
  }
}

Json MeshRenderer::Serialize() {
  Json json;
  json["mesh"]["modelpath"] = meshData->modelPath;
  json["mesh"]["identifier"] = meshData->identifier;
  return json;
}

void MeshRenderer::Deserialize(Json &json) {
  std::string modelPath = json["mesh"]["modelpath"];
  std::string identifier = json["mesh"]["identifier"];
  meshData = Loader.GetMesh(modelPath, identifier);
}

void MeshRenderer::ForwardRender(glm::mat4 projMat, glm::mat4 viewMat,
                                 Entity *camera, Entity *object,
                                 std::vector<Light> &lights) {
  // apply all the deformers before actual rendering
  for (auto deformer : deformers) {
    deformer->DeformMesh(object, true);
  }
  for (auto pass : passes) {
    pass->GetShader()->Use();
    pass->SetupLights(GWORLD.Context.activeLights);
    pass->SetVariables(object->GetModelMatrix(), viewMat, projMat,
                       -camera->LocalForward);
    meshData->Draw(*pass->GetShader());
  }
}

void MeshRenderer::DrawInspectorGUI() {
  if (ImGui::TreeNode("MeshRenderer")) {
    ImGui::MenuItem("Options", nullptr, nullptr, false);
    if (ImGui::TreeNode("Deformers")) {
      for (auto deformer : deformers) {
        ImGui::Separator();
        deformer->DrawInspectorGUI();
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("Render Passes")) {
      for (auto pass : passes) {
        ImGui::Separator();
        pass->DrawInspectorGUI();
      }
      ImGui::TreePop();
    }
    ImGui::TreePop();
  }
}

std::vector<glm::mat4> Animator::GetSkeletonTransforms() {
  std::vector<glm::mat4> result(CurrentPose.skeleton->GetNumJoints(),
                                glm::mat4(1.0f));
  // capture position, rotation of joints, convert these information into
  // matrices

  return result;
}

}; // namespace aEngine