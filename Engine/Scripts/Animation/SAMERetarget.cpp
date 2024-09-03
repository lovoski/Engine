#include "Scripts/Animation/SAMERetarget.hpp"

namespace aEngine {

using asio::ip::tcp;

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
                           }
                         });
}

void SAMERetarget::handleLoadMotion(std::string motionPath) {
  Animation::Motion retargetedMotion;
  motionPath.pop_back();
  if (fs::path(motionPath).extension().string() == ".bvh") {
    retargetedMotion.LoadFromBVH(motionPath);
    LOG_F(INFO, "retarget motion has %d joints", retargetedMotion.skeleton.GetNumJoints());
  } else {
    LOG_F(ERROR, "retarget motion must be a .bvh file");
  }
}

void SAMERetarget::LateUpdate(float dt) {
  auto animator = entity->GetComponent<Animator>();
  if (animator != nullptr && motion != nullptr) {
    // apply motion to animator
    auto animSystem = GWORLD.GetSystemInstance<AnimationSystem>();
    auto currentFrame = animSystem->SystemCurrentFrame;
    auto currentPose = motion->At(currentFrame);
    animator->ApplyMotionToSkeleton(currentPose);
  }
}

void SAMERetarget::DrawInspectorGUI() {
  auto animator = entity->GetComponent<Animator>();
  drawInspectorGUIDefault();
  ImGui::MenuItem("Network", nullptr, nullptr, false);
  if (ImGui::Button("Test Connect", {-1, 30})) {
    Connect();
  }
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
        motion = Loader.GetMotion(filepath.string());
        motionName = filepath.string();
      } else {
        LOG_F(ERROR, "only .bvh and .fbx motion are supported");
      }
    }
    ImGui::EndDragDropTarget();
  }
}

}; // namespace aEngine