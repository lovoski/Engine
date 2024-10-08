/**
 * Neural retargeting with "SAME: Skeleton-Agnostic Motion Embedding for
 * Character Animation" Do the main inference in a python server, this is the
 * c++ client component that handles the user interaction and motion
 * post-processing.
 */
#pragma once

#include "API.hpp"

#include <boost/asio.hpp>

namespace aEngine {

struct SAMERetarget : public Scriptable {
  SAMERetarget() : socket(context) {}

  // Motion data related
  std::string motionName = "";
  Animation::Motion *motion = nullptr;
  std::shared_ptr<Animation::Motion> sourceMotion = nullptr;

  // Network related
  boost::asio::io_context context;
  boost::asio::ip::tcp::socket socket;
  std::string server = "127.0.0.1", port = "9999";
  std::string responseBuffer, sendBuffer;

  void Update(float dt) override { context.poll(); }
  void LateUpdate(float dt) override;

  void Connect();

private:
  void resetMotionVariables();
  void sendDataToServer();
  void receiveDataFromServer();
  void handleLoadMotion(std::string motionPath);
  // rename the jointNames of source to fit target,
  // convert motion of source to target format
  void fitRetargetMotion(Animation::Motion *source,
                         Animation::Skeleton *target);

  void DrawInspectorGUI() override;

  std::string getInspectorWindowName() override { return "SAME Retarget"; }
};

}; // namespace aEngine