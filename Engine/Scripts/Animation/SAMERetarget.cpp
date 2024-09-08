#include "Scripts/Animation/SAMERetarget.hpp"

namespace aEngine {

using asio::ip::tcp;

void SAMERetarget::resetMotionVariables() {
  motion = nullptr;
  motionName = "";
  sourceMotion = nullptr;
}

void SAMERetarget::Connect() {
  tcp::resolver resolver(context);
  auto endPoints = resolver.resolve(server, port);
  asio::async_connect(socket, endPoints,
                      [this](std::error_code ec, tcp::endpoint) {
                        if (!ec) {
                          LOG_F(INFO, "Successfully connect to server %s:%s",
                                server.c_str(), port.c_str());
                          sendDataToServer();
                        } else {
                          LOG_F(ERROR, "Failed to connect to server %s:%s",
                                server.c_str(), port.c_str());
                          resetMotionVariables();
                        }
                      });
}

void SAMERetarget::sendDataToServer() {
  auto actor = entity->GetComponent<Animator>()->actor;
  std::string exportSkeletonFilepath = "./tmp_skeleton.bvh";
  std::string exportMotionFilepath = "./tmp_motion.bvh";
  actor->ExportAsBVH(exportSkeletonFilepath, false);
  sourceMotion->SaveToBVH(exportMotionFilepath);
  sendBuffer = fs::canonical(exportSkeletonFilepath).string();
  sendBuffer = sendBuffer + ";" + fs::canonical(exportMotionFilepath).string();
  asio::async_write(socket, asio::buffer(sendBuffer),
                    [this](std::error_code ec, std::size_t length) {
                      if (!ec) {
                        LOG_F(INFO, "send data to server");
                        receiveDataFromServer();
                        sendBuffer = "";
                      } else {
                        LOG_F(ERROR, "can't send data to server");
                        resetMotionVariables();
                        sendBuffer = "";
                      }
                    });
}
void SAMERetarget::receiveDataFromServer() {
  asio::async_read_until(socket, asio::dynamic_buffer(responseBuffer), ';',
                         [this](std::error_code ec, std::size_t length) {
                           if (!ec) {
                             LOG_F(INFO,
                                   "receive data from server %s, length=%d",
                                   responseBuffer.c_str(), length);
                             handleLoadMotion(responseBuffer);
                             responseBuffer = "";
                           } else {
                             LOG_F(ERROR, "can't receive data from server");
                             resetMotionVariables();
                           }
                         });
}

void SAMERetarget::fitRetargetMotion(Animation::Motion *source,
                                     Animation::Skeleton *target) {
  int sourceJointNum = source->skeleton.GetNumJoints();
  int targetJointNum = target->GetNumJoints();
  if (sourceJointNum != targetJointNum) {
    LOG_F(ERROR,
          "the joint of source skeleton and target skeleton must be the same");
    return;
  }
  int jointNum = sourceJointNum;
  auto sourceGlobalPosition =
      source->skeleton.GetRestPose().GetGlobalPositions();
  std::vector<glm::quat> targetJointOrien;
  auto targetGlobalPosition =
      target->GetRestPose().GetGlobalPositionOrientation(targetJointOrien);
  // the result of same will match the target skeleton in order but not in names
  source->skeleton.jointNames = target->jointNames;
  // fit the motion data to target skeleton format if the target skeleton is
  // fbx-style
  bool fbxStyleTarget = false;
  for (int i = 0; i < jointNum; ++i) {
    if (target->jointRotation[i] != glm::quat(1.0f, glm::vec3(0.0f))) {
      fbxStyleTarget = true; // bvh-style skeleton don't have rotation
      break;
    }
  }
  if (fbxStyleTarget) {
    // the source motion is bvh-style, apply global rotation at each frame
    int numFrames = source->poses.size();
    std::vector<glm::quat> oldOrien,
        newOrien(jointNum, glm::quat(1.0f, glm::vec3(0.0f)));
    for (int frameInd = 0; frameInd < numFrames; ++frameInd) {
      source->poses[frameInd].GetGlobalPositionOrientation(oldOrien);
      for (int jointInd = 0; jointInd < jointNum; ++jointInd) {
        // oldOrien = newOrien * inv(initialOrien)
        newOrien[jointInd] = oldOrien[jointInd] * targetJointOrien[jointInd];
      }
      // build local rotations from global orientations
      glm::quat parentOrien;
      for (int jointInd = 0; jointInd < jointNum; ++jointInd) {
        int parentInd = source->skeleton.jointParent[jointInd];
        if (parentInd == -1)
          parentOrien = glm::quat(1.0f, glm::vec3(0.0f));
        else
          parentOrien = newOrien[parentInd];
        source->poses[frameInd].jointRotations[jointInd] =
            glm::inverse(parentOrien) * newOrien[jointInd];
      }
    }
  }
}

void SAMERetarget::handleLoadMotion(std::string motionPath) {
  motionPath.pop_back();
  if (fs::path(motionPath).extension().string() == ".bvh") {
    auto actor = entity->GetComponent<Animator>()->actor;
    auto retargetedMotion = Loader.GetMotion(motionPath);
    if (retargetedMotion) {
      // setup retarget motion
      // this is a async function called from Update function,
      // so we need to build skeleton map manually for this new motion
      fitRetargetMotion(retargetedMotion,
                        entity->GetComponent<Animator>()->actor);
      motion = retargetedMotion;
    } else {
      LOG_F(ERROR, "retarget motion not valid");
      resetMotionVariables();
    }
  } else {
    LOG_F(ERROR, "retarget motion must be a .bvh file, gets %s",
          motionPath.c_str());
    resetMotionVariables();
  }
}

void SAMERetarget::LateUpdate(float dt) {
  auto animator = entity->GetComponent<Animator>();
  if (animator != nullptr && motion != nullptr) {
    // apply motion to animator
    auto animSystem = GWORLD.GetSystemInstance<AnimationSystem>();
    auto currentFrame = animSystem->SystemCurrentFrame;
    auto currentPose = motion->At(currentFrame);
    // apply the retarget motion
    animator->ApplyMotionToSkeleton(currentPose);
  }
}

void SAMERetarget::DrawInspectorGUI() {
  auto animator = entity->GetComponent<Animator>();
  drawInspectorGUIDefault();
  ImGui::MenuItem("Motion", nullptr, nullptr, false);
  ImGui::TextWrapped("FPS: %d", motion == nullptr ? -1 : motion->fps);
  ImGui::TextWrapped("Duration: %d",
                     motion == nullptr ? -1 : motion->poses.size());
  if (ImGui::Button("Export BVH Motion##same", {-1, 30})) {
    if (motion != nullptr)
      motion->SaveToBVH("./save_motion.bvh");
  }
  ImGui::BeginChild("choosesamemotionsource", {-1, 30});
  static char motionSequencePath[200] = {0};
  sprintf(motionSequencePath, motionName.c_str());
  ImGui::InputTextWithHint("##samemotionsource", "Motion Sequence Path",
                           motionSequencePath, sizeof(motionSequencePath),
                           ImGuiInputTextFlags_ReadOnly);
  ImGui::SameLine();
  if (ImGui::Button("Clear##same", {-1, -1})) {
    // reset skeleton to rest pose
    if (animator != nullptr)
      animator->ApplyMotionToSkeleton(animator->actor->GetRestPose());
    // clear variables
    resetMotionVariables();
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
      if (extension == ".bvh") {
        // TODO: currently, only bvh file are supported as source motion
        sourceMotion = std::make_shared<Animation::Motion>();
        sourceMotion->LoadFromBVH(filepath.string());
        // try retarget this motion to current animator's actor
        if (animator != nullptr) {
          LOG_F(INFO, "Actor joint number: %d, motion joint number %d",
                animator->actor->GetNumJoints(),
                sourceMotion->skeleton.GetNumJoints());
          Connect();
          motionName = filepath.string();
        } else {
          LOG_F(ERROR,
                "Can't do motion retarget without an Animator component");
        }
      } else {
        LOG_F(ERROR, "only .bvh and .fbx motion are supported");
      }
    }
    ImGui::EndDragDropTarget();
  }
}

}; // namespace aEngine