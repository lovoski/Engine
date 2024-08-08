#include "resource/animation/Motion.hpp"

#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
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

// rotation channel order:
// XYZ -> 012
// XZY -> 021
// YXZ -> 102
// YZX -> 120
// ZXY -> 201
// ZYX -> 210
// NONE -> 0
vec3 xaxis(1.0f, 0.0f, 0.0f);
vec3 yaxis(0.0f, 1.0f, 0.0f);
vec3 zaxis(0.0f, 0.0f, 1.0f);

quat QuatFromEulers(float a, float b, float c, int order) {
  if (order == 12) {
    return glm::angleAxis(c, zaxis) * glm::angleAxis(b, yaxis) *
           glm::angleAxis(a, xaxis);
  } else if (order == 21) {
    return glm::angleAxis(c, yaxis) * glm::angleAxis(b, zaxis) *
           glm::angleAxis(a, xaxis);
  } else if (order == 102) {
    return glm::angleAxis(c, zaxis) * glm::angleAxis(b, xaxis) *
           glm::angleAxis(a, yaxis);
  } else if (order == 120) {
    return glm::angleAxis(c, xaxis) * glm::angleAxis(b, zaxis) *
           glm::angleAxis(a, yaxis);
  } else if (order == 201) {
    return glm::angleAxis(c, yaxis) * glm::angleAxis(b, xaxis) *
           glm::angleAxis(a, zaxis);
  } else if (order == 210) {
    return glm::angleAxis(c, xaxis) * glm::angleAxis(b, yaxis) *
           glm::angleAxis(a, zaxis);
  } else
    return quat(1.0f, 0.0f, 0.0f, 0.0f);
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
          if (lineSeg.size() == 0)
            continue; // skip blank lines
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
              skeleton.jointChannels.push_back(numChannels);
              // process the channels, assume that the rotation channels
              // are always behind the position channels
              if (numChannels == 3) {
                // only rotation channels
                int a = lineSeg[2][0] - 'X';
                int b = lineSeg[3][0] - 'X';
                int c = lineSeg[4][0] - 'X';
                skeleton.jointChannelsOrder.push_back(a * 100 + b * 10 + c);
              } else if (numChannels == 6) {
                // position + rotation channels
                int a = lineSeg[5][0] - 'X';
                int b = lineSeg[6][0] - 'X';
                int c = lineSeg[7][0] - 'X';
                skeleton.jointChannelsOrder.push_back(a * 100 + b * 10 + c);
              } else
                throw std::runtime_error(
                    "number of channels in bvh must be either 3 or 6");
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
            skeleton.jointNames.push_back(skeleton.jointNames[parentJoint] +
                                          "_End"); // the end effector's name
            getline(fileInput, line);              // {
            getline(fileInput, line);              // OFFSET
            lineSeg = SplitByWhiteSpace(line);
            if (lineSeg[0] == "OFFSET") {
              float xOffset = std::stof(lineSeg[1]);
              float yOffset = std::stof(lineSeg[2]);
              float zOffset = std::stof(lineSeg[3]);
              skeleton.jointOffset.push_back(vec3(xOffset, yOffset, zOffset));
              skeleton.jointChildren.push_back(vector<int>());
              skeleton.jointChannels.push_back(0); // 0 for end effector
              skeleton.jointChannelsOrder.push_back(0);
            } else
              throw std::runtime_error(
                  "the label should be OFFSET for end effector");
            getline(fileInput, line); // }
            currentJoint++;           // move to the next joint index
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
              fps = (int)(1.0f / timePerFrame);
              const int jointNumber = skeleton.GetNumJoints();
              for (int frameInd = 0; frameInd < poses.size(); ++frameInd) {
                getline(fileInput, line);
                lineSeg = SplitByWhiteSpace(line);
                poses[frameInd].skeleton = &this->skeleton;
                poses[frameInd].jointPositions.resize(jointNumber, vec3(0.0f));
                poses[frameInd].jointRotations.resize(jointNumber, vec3(0.0f));
                int segInd = 0;
                for (int jointInd = 0; jointInd < jointNumber; ++jointInd) {
                  if (skeleton.jointChannels[jointInd] == 6) {
                    // set up the positions if exists
                    float x = std::stof(lineSeg[segInd++]),
                          y = std::stof(lineSeg[segInd++]),
                          z = std::stof(lineSeg[segInd++]);
                    poses[frameInd].jointPositions[jointInd] = vec3(x, y, z);
                  }
                  if (skeleton.jointChannels[jointInd] != 0) {
                    // set up rotations
                    float a = glm::radians(std::stof(lineSeg[segInd++])),
                          b = glm::radians(std::stof(lineSeg[segInd++])),
                          c = glm::radians(std::stof(lineSeg[segInd++]));
                    int rotationOrder = skeleton.jointChannelsOrder[jointInd];
                    poses[frameInd].jointRotations[jointInd] = vec3(a, b, c);
                  } else {
                    // set the rotation of end effectors to normal quaternion
                    poses[frameInd].jointRotations[jointInd] = vec3(0.0f);
                  }
                }
              }
            }
          } else
            throw std::runtime_error("number of frames must be specified");
        } else
          throw std::runtime_error(
              "pose data should start with a MOTION label");
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

inline void BVHPadding(std::ostream &out, int depth) {
  for (int i = 0; i < depth; ++i)
    out << "\t";
}

inline void OrderedOutput(std::ofstream &out, vec3 euler, int order) {
  if (order == 12) {
    out << euler.x << " " << euler.y << " " << euler.z << " ";
  } else if (order == 21) {
    out << euler.x << " " << euler.z << " " << euler.y << " ";
  } else if (order == 102) {
    out << euler.y << " " << euler.x << " " << euler.z << " ";
  } else if (order == 120) {
    out << euler.y << " " << euler.z << " " << euler.x << " ";
  } else if (order == 201) {
    out << euler.z << " " << euler.x << " " << euler.y << " ";
  } else if (order == 210) {
    out << euler.z << " " << euler.y << " " << euler.x << " ";
  }
}

inline void OrderedOutput(std::ofstream &out, int order) {
  if (order == 12) {
    out << "Xrotation Yrotation Zrotation\n";
  } else if (order == 21) {
    out << "Xrotation Zrotation Yrotation\n";
  } else if (order == 102) {
    out << "Yrotation Xrotation Zrotation\n";
  } else if (order == 120) {
    out << "Yrotation Zrotation Xrotation\n";
  } else if (order == 201) {
    out << "Zrotation Xrotation Yrotation\n";
  } else if (order == 210) {
    out << "Zrotation Yrotation Xrotation\n";
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
    // write the skeleton hierarchy
    int depth = 0;
    for (int jointInd = 0; jointInd < skeleton.GetNumJoints(); ++jointInd) {
      BVHPadding(fileOutput, depth);
      if (skeleton.jointChildren[jointInd].size() == 0) {
        fileOutput << "End Site\n";
        BVHPadding(fileOutput, depth++);
        fileOutput << "{\n";
        BVHPadding(fileOutput, depth);
        fileOutput << "OFFSET " << std::fixed << std::setprecision(4)
                   << skeleton.jointOffset[jointInd].x << " "
                   << skeleton.jointOffset[jointInd].y << " "
                   << skeleton.jointOffset[jointInd].z << "\n";
        BVHPadding(fileOutput, --depth);
        fileOutput << "}\n";
        // find the direct parent of the next joint (if exists)
        if (jointInd == skeleton.GetNumJoints()-1) {
          // flush until depth is 0
          while (depth > 0) {
            BVHPadding(fileOutput, --depth);
            fileOutput << "}\n";
          }
        } else {
          int parentOfNext = skeleton.jointParent[jointInd+1];
          int cur = skeleton.jointParent[jointInd];
          while (cur != parentOfNext) {
            BVHPadding(fileOutput, --depth);
            fileOutput << "}\n";
            cur = skeleton.jointParent[cur];
          }
        }
      } else {
        if (skeleton.jointParent[jointInd] == -1)
          fileOutput << "ROOT " << skeleton.jointNames[jointInd] << "\n";
        else
          fileOutput << "JOINT " << skeleton.jointNames[jointInd] << "\n";
        BVHPadding(fileOutput, depth++);
        fileOutput << "{\n";
        BVHPadding(fileOutput, depth);
        fileOutput << "OFFSET " << std::fixed << std::setprecision(4)
                   << skeleton.jointOffset[jointInd].x << " "
                   << skeleton.jointOffset[jointInd].y << " "
                   << skeleton.jointOffset[jointInd].z << "\n";
        BVHPadding(fileOutput, depth);
        if (skeleton.jointChannels[jointInd] == 3) {
          fileOutput << "CHANNELS " << skeleton.jointChannels[jointInd] << " ";
          OrderedOutput(fileOutput, skeleton.jointChannelsOrder[jointInd]);
        } else {
          fileOutput << "CHANNELS " << skeleton.jointChannels[jointInd]
                     << " Xposition Yposition Zposition ";
          OrderedOutput(fileOutput, skeleton.jointChannelsOrder[jointInd]);
        }
      }
    }
    fileOutput.flush();

    // write the pose data
    fileOutput << "MOTION\nFrames: " << poses.size() << "\n"
               << "Frame Time: " << 1.0f / fps << "\n";
    for (int frameInd = 0; frameInd < poses.size(); ++frameInd) {
      for (int jointInd = 0; jointInd < skeleton.GetNumJoints(); ++jointInd) {
        if (skeleton.jointChannels[jointInd] != 0) {
          // XYZ positions, ZXY rotations
          if (skeleton.jointChannels[jointInd] == 6) {
            fileOutput << poses[frameInd].jointPositions[jointInd].x << " "
                       << poses[frameInd].jointPositions[jointInd].y << " "
                       << poses[frameInd].jointPositions[jointInd].z << " ";
          }
          vec3 euler = poses[frameInd].jointRotations[jointInd];
          euler = glm::degrees(euler);
          OrderedOutput(fileOutput, euler, skeleton.jointChannelsOrder[jointInd]);
        }
      }
      fileOutput << "\n";
    }
    fileOutput.close();
    return true;
  }
}

}; // namespace Resource