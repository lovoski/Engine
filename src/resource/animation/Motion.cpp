#include "resource/animation/Motion.hpp"

#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stack>

namespace Resource {

using glm::cross;
using glm::dot;
using glm::quat;
using glm::vec3;
using std::getline;
using std::stack;
using std::string;
using std::vector;

inline bool IsWhiteSpace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

// split a string by white space
vector<string> SplitByWhiteSpace(string str) {
  int last = 0, cur = 0;
  vector<string> result;
  while (cur < str.size() && IsWhiteSpace(str[cur]))
    cur++;
  last = cur;
  while (cur <= str.size()) {
    if (cur == str.size() || IsWhiteSpace(str[cur])) {
      if (cur > last)
        result.push_back(str.substr(last, cur - last));
      while (cur < str.size() && IsWhiteSpace(str[cur]))
        cur++;
      last = cur;
    }
    cur++;
  }
  return result;
}

bool Motion::LoadFromBVH(string filename) {
  std::ifstream fileInput(filename);
  if (!fileInput.is_open()) {
    printf("failed to open file %s\n", filename.c_str());
    fileInput.close();
    return false;
  } else {
    skeleton.skeletonName = std::filesystem::path(filename).stem().string();
    string line;
    getline(fileInput, line);
    if (line == "HIERARCHY") {
      getline(fileInput, line);
      auto lineSeg = SplitByWhiteSpace(line);
      if (lineSeg[0] == "ROOT") {
        int currentJoint = 0, parentJoint = -1;
        stack<int> s;
        s.push(currentJoint);
        skeleton.jointNames.push_back(lineSeg[1]);   // the name
        skeleton.jointParent.push_back(parentJoint); // the parent
        while (!s.empty()) {
          getline(fileInput, line);
          lineSeg = SplitByWhiteSpace(line);
          if (lineSeg[0] == "{") {
            currentJoint++;
          } else if (lineSeg[0] == "}") {
            s.pop();
          } else if (lineSeg[0] == "OFFSET") {
            float xOffset = std::stof(lineSeg[1]);
            float yOffset = std::stof(lineSeg[2]);
            float zOffset = std::stof(lineSeg[3]);
            skeleton.jointOffset.push_back(vec3(xOffset, yOffset, zOffset));
            // ready to recieve children
            skeleton.jointChildren.push_back(vector<int>());
            getline(fileInput, line);
            lineSeg = SplitByWhiteSpace(line);
            if (lineSeg[0] == "CHANNELS") {
              int numChannels = std::stoi(lineSeg[1]);
              // skeleton.jointChannels.push_back(numChannels);
              // TODO: process the channels
              // for (int cIndex = 0; cIndex < numChannels; ++cIndex);
            } else
              throw std::runtime_error(
                  ("this label should be CHANNELS instead of " + lineSeg[0])
                      .c_str());
          } else if (lineSeg[0] == "JOINT") {
            parentJoint = s.top();
            skeleton.jointParent.push_back(parentJoint);
            skeleton.jointChildren[parentJoint].push_back(currentJoint);
            skeleton.jointNames.push_back(lineSeg[1]); // name of this joint
            s.push(currentJoint); // keep record of this child joint
          } else if (lineSeg[0] == "End") {
            parentJoint = s.top();
            skeleton.jointParent.push_back(parentJoint);
            skeleton.jointChildren[parentJoint].push_back(currentJoint);
            skeleton.jointNames.push_back(
                skeleton.jointNames[parentJoint] +
                "_End");              // the end effector's name
            getline(fileInput, line); // {
            getline(fileInput, line); // OFFSET
            lineSeg = SplitByWhiteSpace(line);
            if (lineSeg[0] == "OFFSET") {
              float xOffset = std::stof(lineSeg[1]);
              float yOffset = std::stof(lineSeg[2]);
              float zOffset = std::stof(lineSeg[3]);
              skeleton.jointOffset.push_back(vec3(xOffset, yOffset, zOffset));
              skeleton.jointChildren.push_back(vector<int>());
              // skeleton.jointChannels.push_back(0); // 0 for end effector
            } else
              throw std::runtime_error(
                  "the label should be OFFSET for end effector");
            getline(fileInput, line); // }
            currentJoint++; // move to the next joint index
          }
        }

        // parse pose data
        getline(fileInput, line);
        lineSeg = SplitByWhiteSpace(line);
        if (lineSeg[0] == "MOTION") {
          getline(fileInput, line);
          lineSeg = SplitByWhiteSpace(line);
          if (lineSeg[0] == "Frames:") {
            poses.resize(std::stoi(lineSeg[1]));
            getline(fileInput, line);
            lineSeg = SplitByWhiteSpace(line);
            if (lineSeg[0] == "Frame" && lineSeg[1] == "Time:") {
              float timePerFrame = std::stof(lineSeg[2]);
              fps = (int)(1.0f/timePerFrame);
              const int jointNumber = skeleton.GetNumJoints();
              for (int frameInd = 0; frameInd < poses.size(); ++frameInd) {
                getline(fileInput, line);
                lineSeg = SplitByWhiteSpace(line);
                poses[frameInd].skeleton = &skeleton;
                int segInd = 0;
                for (int jointInd = 0; jointInd < jointNumber; ++jointInd) {
                  // for (int channelInd = 0; channelInd < skeleton.jointChannels[jointInd]; ++channelInd) {
                  //   // set up value of each channel
                  // }
                }
              }
            }
          } else throw std::runtime_error("number of frames must be specified");
        } else throw std::runtime_error("pose data should start with a MOTION label");
        fileInput.close();
        return true;
      } else
        throw std::exception(
            ("the label should be ROOT instead of " + lineSeg[0]).c_str());
    } else
      throw std::exception("bvh file should start with HIERARCHY");
    fileInput.close();
    return false;
  }
}

bool Motion::SaveToBVH(string filename) {
  std::ofstream fileOutput(filename);
  if (!fileOutput.is_open()) {
    printf("failed to open file %s\n", filename.c_str());
    fileOutput.close();
    return false;
  } else {
    fileOutput << "HIERARCHY" << std::endl;

  }
}

}; // namespace Resource