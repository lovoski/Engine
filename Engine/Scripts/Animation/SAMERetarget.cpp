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

void SAMERetarget::handleLoadMotion(std::string motionPath) {
  motionPath.pop_back();
  if (fs::path(motionPath).extension().string() == ".bvh") {
    auto actor = entity->GetComponent<Animator>()->actor;
    auto retargetedMotion = Loader.GetMotion(motionPath);
    if (retargetedMotion) {
      // setup retarget motion
      // motion = retargetedMotion;
      auto motionViewer = GWORLD.AddNewEntity();
      motionViewer->name = fs::path(motionPath).filename().string();
      motionViewer->AddComponent<Animator>(retargetedMotion);
      motionViewer->GetComponent<Animator>()->motionName = motionPath;
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
  ImGui::BeginChild("choosemotionsource", {-1, 30});
  static char motionSequencePath[200] = {0};
  sprintf(motionSequencePath, motionName.c_str());
  ImGui::InputTextWithHint("##motionsource", "Motion Sequence Path",
                           motionSequencePath, sizeof(motionSequencePath),
                           ImGuiInputTextFlags_ReadOnly);
  ImGui::SameLine();
  if (ImGui::Button("Clear", {-1, -1})) {
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
      if (extension == ".bvh" || extension == ".fbx") {
        sourceMotion = Loader.GetMotion(filepath.string());
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