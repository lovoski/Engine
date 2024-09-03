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
  std::string message = "Hello from C++ client";
  asio::async_write(socket, asio::buffer(message),
                    [this](std::error_code ec, std::size_t length) {
                      if (!ec) {
                        LOG_F(INFO, "send data to server");
                        receiveDataFromServer();
                      } else {
                        LOG_F(ERROR, "can't send data to server");
                        resetMotionVariables();
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
    motion = Loader.GetMotion(motionPath);
    if (!motion) {
      LOG_F(ERROR, "can't load retarget motion %s", motionPath.c_str());
      resetMotionVariables();
    }
  } else {
    LOG_F(ERROR, "retarget motion must be a .bvh file, gets %s", motionPath.c_str());
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
    motionName = "";
    motion = nullptr;
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
        auto motionToBeRetarget = Loader.GetMotion(filepath.string());
        // try retarget this motion to current animator's actor
        if (animator != nullptr) {
          LOG_F(INFO, "Actor joint number: %d, motion joint number %d",
                animator->actor->GetNumJoints(),
                motionToBeRetarget->skeleton.GetNumJoints());
          Connect();
          motionName = filepath.string();
        } else {
          LOG_F(ERROR, "Can't do motion retarget without an Animator component");
        }
      } else {
        LOG_F(ERROR, "only .bvh and .fbx motion are supported");
      }
    }
    ImGui::EndDragDropTarget();
  }
}

}; // namespace aEngine