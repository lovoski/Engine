#pragma once

#include "API.hpp"
#include <asio.hpp>

namespace aEngine {

struct SAMERetarget : public Scriptable {
  SAMERetarget() : socket(context) {}

  // Motion data related
  std::string motionName = "";
  Animation::Motion *motion = nullptr;

  // Network related
  asio::io_context context;
  asio::ip::tcp::socket socket;
  std::string server = "127.0.0.1", port = "9999";
  std::string responseBuffer;

  void Update(float dt) override { context.poll(); }
  void LateUpdate(float dt) override;

  void DrawInspectorGUI() override;

  void Connect();

private:
  void sendDataToServer();
  void receiveDataFromServer();
  void handleLoadMotion(std::string motionPath);

  std::string getTypeName() override { return "SAME Retarget"; }
};

}; // namespace aEngine