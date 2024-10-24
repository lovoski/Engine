#include "Function/Animation/Motion.hpp"
#include "Function/Math/Math.hpp"

#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <stack>

using glm::cross;
using glm::dot;
using glm::quat;
using glm::vec3;
using std::getline;
using std::stack;
using std::string;
using std::vector;

namespace aEngine {

namespace Animation {

inline bool IsWhiteSpace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

inline bool EndsWith(string target, string pattern) {
  int pointer = 0, targetSize = target.size(), patternSize = pattern.size();
  if (targetSize < patternSize)
    return false;
  while (pointer < patternSize) {
    if (target[targetSize - 1 - pointer] != pattern[patternSize - 1 - pointer])
      return false;
    pointer++;
  }
  return true;
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

#define EULER_XYZ 12
#define EULER_XZY 21
#define EULER_YXZ 102
#define EULER_YZX 120
#define EULER_ZXY 201
#define EULER_ZYX 210

quat QuatFromEulers(vec3 angles, int order) {
  quat qx = glm::angleAxis(angles.x, vec3(1.0f, 0.0f, 0.0f));
  quat qy = glm::angleAxis(angles.y, vec3(0.0f, 1.0f, 0.0f));
  quat qz = glm::angleAxis(angles.z, vec3(0.0f, 0.0f, 1.0f));
  if (order == EULER_XYZ) {
    return qx * qy * qz;
  } else if (order == EULER_XZY) {
    return qx * qz * qy;
  } else if (order == EULER_YXZ) {
    return qy * qx * qz;
  } else if (order == EULER_YZX) {
    return qy * qz * qx;
  } else if (order == EULER_ZXY) {
    return qz * qx * qy;
  } else if (order == EULER_ZYX) {
    return qz * qy * qx;
  } else
    return quat(1.0f, vec3(0.0f));
}

bool Motion::LoadFromBVH(string filename, float scale) {
  std::ifstream fileInput(filename);
  if (!fileInput.is_open()) {
    printf("failed to open file %s\n", filename.c_str());
    fileInput.close();
    return false;
  } else {
    skeleton.skeletonName = std::filesystem::path(filename).stem().string();
    vector<int> jointChannels;
    vector<int> jointChannelsOrder;
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
            float xOffset = std::stof(lineSeg[1]) * scale;
            float yOffset = std::stof(lineSeg[2]) * scale;
            float zOffset = std::stof(lineSeg[3]) * scale;
            skeleton.jointOffset.push_back(vec3(xOffset, yOffset, zOffset));
            // ready to recieve children
            skeleton.jointChildren.push_back(vector<int>());
            getline(fileInput, line);
            lineSeg = SplitByWhiteSpace(line);
            if (lineSeg[0] == "CHANNELS") {
              int numChannels = std::stoi(lineSeg[1]);
              jointChannels.push_back(numChannels);
              // process the channels, assume that the rotation channels
              // are always behind the position channels
              if (numChannels == 3 || numChannels == 6) {
                // only rotation channels
                int a = lineSeg[numChannels - 1][0] - 'X';
                int b = lineSeg[numChannels][0] - 'X';
                int c = lineSeg[numChannels + 1][0] - 'X';
                jointChannelsOrder.push_back(a * 100 + b * 10 + c);
              } else
                throw std::runtime_error(
                    "number of channels in bvh must be either 3 or 6");
            } else if (lineSeg[0] == "}") {
              jointChannels.push_back(0);
              jointChannelsOrder.push_back(0);
              s.pop(); // this is a end effector
            } else
              throw std::runtime_error(
                  ("this label should be CHANNELS or `}` instead of " +
                   lineSeg[0])
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
              float xOffset = std::stof(lineSeg[1]) * scale;
              float yOffset = std::stof(lineSeg[2]) * scale;
              float zOffset = std::stof(lineSeg[3]) * scale;
              skeleton.jointOffset.push_back(vec3(xOffset, yOffset, zOffset));
              skeleton.jointChildren.push_back(vector<int>());
              jointChannels.push_back(0); // 0 for end effector
              jointChannelsOrder.push_back(0);
            } else
              throw std::runtime_error(
                  "the label should be OFFSET for end effector");
            getline(fileInput, line); // }
            currentJoint++;           // move to the next joint index
          }
        }

        // initialize jointRotation and jointScale
        // bvh format don't store localRotation and localScale
        // of a skeleton hierarchy, initialize to identity transform
        skeleton.jointRotation = std::vector<glm::quat>(
            skeleton.GetNumJoints(), glm::quat(1.0f, vec3(0.0f)));
        skeleton.jointScale =
            std::vector<glm::vec3>(skeleton.GetNumJoints(), glm::vec3(1.0f));

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
              fps = glm::round(1.0f / timePerFrame);
              const int jointNumber = skeleton.GetNumJoints();
              for (int frameInd = 0; frameInd < poses.size(); ++frameInd) {
                getline(fileInput, line);
                lineSeg = SplitByWhiteSpace(line);
                vector<vec3> jointPositions(jointNumber, vec3(0.0f));
                poses[frameInd].skeleton = &this->skeleton;
                poses[frameInd].jointRotations.resize(jointNumber, vec3(0.0f));
                int segInd = 0;
                for (int jointInd = 0; jointInd < jointNumber; ++jointInd) {
                  if (jointChannels[jointInd] == 6) {
                    // set up the positions if exists
                    float x = std::stof(lineSeg[segInd++]) * scale,
                          y = std::stof(lineSeg[segInd++]) * scale,
                          z = std::stof(lineSeg[segInd++]) * scale;
                    jointPositions[jointInd] = vec3(x, y, z);
                  }
                  if (jointChannels[jointInd] != 0) {
                    // set up rotations
                    vec3 v(0.0f);
                    int rotationOrder = jointChannelsOrder[jointInd];
                    int o1 = rotationOrder / 100;
                    int o2 = (rotationOrder - o1 * 100) / 10;
                    int o3 = rotationOrder - o1 * 100 - o2 * 10;
                    v[o1] = std::stof(lineSeg[segInd++]);
                    v[o2] = std::stof(lineSeg[segInd++]);
                    v[o3] = std::stof(lineSeg[segInd++]);
                    poses[frameInd].jointRotations[jointInd] =
                        QuatFromEulers(glm::radians(v), rotationOrder);
                  } else {
                    // set the rotation of end effectors to normal quaternion
                    poses[frameInd].jointRotations[jointInd] =
                        quat(1.0f, vec3(0.0f));
                  }
                }
                // setup the root translation only
                poses[frameInd].rootLocalPosition = jointPositions[0];
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
        throw std::runtime_error("the label should be ROOT instead of " +
                                 lineSeg[0]);
    } else
      throw std::runtime_error("bvh file should start with HIERARCHY");
    fileInput.close();
    return false;
  }
}

inline void BVHPadding(std::ostream &out, int depth) {
  for (int i = 0; i < depth; ++i)
    out << "\t";
}

bool Motion::SaveToBVH(string filename, bool keepJointNames, float scale) {
  // apply the initial rotations of skeleton joints
  // the motion data remains unchanged
  auto restPose = skeleton.GetRestPose();
  int jointNumber = skeleton.GetNumJoints();
  vector<quat> globalJointOrien;
  auto globalJointPositions =
      restPose.GetGlobalPositionOrientation(globalJointOrien);
  auto flattenJointOffset = globalJointPositions;
  for (int i = 0; i < skeleton.GetNumJoints(); ++i) {
    int parentInd = skeleton.jointParent[i];
    if (parentInd != -1) {
      flattenJointOffset[i] -= globalJointPositions[parentInd];
    }
  }
  // output the skeleton and motions
  std::ofstream fileOutput(filename);
  if (!fileOutput.is_open()) {
    printf("failed to open file %s\n", filename.c_str());
    fileOutput.close();
    return false;
  } else {
    fileOutput << "HIERARCHY" << std::endl;
    // write the skeleton hierarchy
    std::set<int> incorrectNamedEE;
    int depth = 0;
    for (int jointInd = 0; jointInd < skeleton.GetNumJoints(); ++jointInd) {
      BVHPadding(fileOutput, depth);
      if (skeleton.jointChildren[jointInd].size() == 0) {
        if (EndsWith(skeleton.jointNames[jointInd], "_End") ||
            !keepJointNames) {
          // force rename end effector
          fileOutput << "End Site\n";
        } else {
          incorrectNamedEE.insert(jointInd);
          fileOutput << "JOINT " << skeleton.jointNames[jointInd] << "\n";
        }
        BVHPadding(fileOutput, depth++);
        fileOutput << "{\n";
        BVHPadding(fileOutput, depth);
        fileOutput << "OFFSET " << flattenJointOffset[jointInd].x * scale << " "
                   << flattenJointOffset[jointInd].y * scale << " "
                   << flattenJointOffset[jointInd].z * scale << "\n";
        if (incorrectNamedEE.count(jointInd) != 0) {
          // add an end effector with no offset to the end
          BVHPadding(fileOutput, depth);
          fileOutput << "CHANNELS 3 Zrotation Yrotation Xrotation\n";
          BVHPadding(fileOutput, depth);
          fileOutput << "End Site\n";
          BVHPadding(fileOutput, depth);
          fileOutput << "{\n";
          BVHPadding(fileOutput, depth + 1);
          fileOutput << "OFFSET 0 0 0\n";
          BVHPadding(fileOutput, depth);
          fileOutput << "}\n";
        }
        BVHPadding(fileOutput, --depth);
        fileOutput << "}\n";
        // find the direct parent of the next joint (if exists)
        if (jointInd == skeleton.GetNumJoints() - 1) {
          // flush until depth is 0
          while (depth > 0) {
            BVHPadding(fileOutput, --depth);
            fileOutput << "}\n";
          }
        } else {
          int parentOfNext = skeleton.jointParent[jointInd + 1];
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
        fileOutput << "OFFSET " << flattenJointOffset[jointInd].x * scale << " "
                   << flattenJointOffset[jointInd].y * scale << " "
                   << flattenJointOffset[jointInd].z * scale << "\n";
        BVHPadding(fileOutput, depth);
        if (skeleton.jointParent[jointInd] != -1) {
          fileOutput << "CHANNELS 3 Zrotation Yrotation Xrotation\n";
        } else {
          fileOutput << "CHANNELS 6 Xposition Yposition Zposition Zrotation "
                        "Yrotation Xrotation\n";
        }
      }
    }
    fileOutput.flush();

    // write the pose data
    fileOutput << "MOTION\nFrames: " << poses.size() << "\n"
               << "Frame Time: " << 1.0f / fps << "\n";
    for (int frameInd = 0; frameInd < poses.size(); ++frameInd) {
      fileOutput << poses[frameInd].rootLocalPosition.x * scale << " "
                 << poses[frameInd].rootLocalPosition.y * scale << " "
                 << poses[frameInd].rootLocalPosition.z * scale << " ";
      vector<quat> frameJointRot(jointNumber, quat(1.0f, vec3(0.0f)));
      vector<quat> newOrien(jointNumber, quat(1.0f, vec3(0.0f)));
      vector<quat> oldOrien;
      // oldOrien is the ground truth rotation we need
      poses[frameInd].GetGlobalPositionOrientation(oldOrien);
      for (int jointInd = 0; jointInd < jointNumber; ++jointInd) {
        int parentInd = skeleton.jointParent[jointInd];
        // `newOrien` is the delta rotation in each frame
        // oldOrien = newOrien * globalJointOrien
        // meaning that we can get the ground truth rotation with the skeleton
        // initial rotation and a delta rotation stored in each frame,
        // we will get local rotation of each joint from this delta global
        // rotation
        newOrien[jointInd] =
            oldOrien[jointInd] * glm::inverse(globalJointOrien[jointInd]);
        if (parentInd == -1) {
          frameJointRot[jointInd] = newOrien[jointInd];
        } else {
          frameJointRot[jointInd] =
              glm::inverse(newOrien[parentInd]) * newOrien[jointInd];
        }
      }
      for (int jointInd = 0; jointInd < jointNumber; ++jointInd) {
        if (skeleton.jointChildren[jointInd].size() != 0) {
          vec3 euler = glm::eulerAngles(frameJointRot[jointInd]);
          vec3 eulerDegree = glm::degrees(euler);
          fileOutput << eulerDegree.z << " " << eulerDegree.y << " "
                     << eulerDegree.x << " ";
        } else {
          if (incorrectNamedEE.count(jointInd) != 0) {
            // if this joint is a incorrectly named ee
            fileOutput << "0 0 0 ";
          }
        }
      }
      fileOutput << "\n";
    }
    fileOutput.close();
    return true;
  }
}

vector<vec3> Pose::GetGlobalPositionOrientation(vector<quat> &orientations) {
  orientations.clear();
  int jointNum = skeleton->GetNumJoints();
  orientations = vector<quat>(jointNum, quat(1.0f, vec3(0.0f)));
  vector<vec3> positions(jointNum, vec3(0.0f));
  if (jointNum != jointRotations.size()) {
    throw std::runtime_error(
        "inconsistent joint number between skeleton and pose data");
    return positions;
  }
  quat parentOrientation;
  vec3 parentPosition;
  // start traversaling the joints
  for (int curJoint = 0; curJoint < jointNum; ++curJoint) {
    // compute the orientation
    int parentInd = skeleton->jointParent[curJoint];
    if (parentInd == -1) {
      parentOrientation = orientations[0];
      parentPosition = glm::vec3(0.0f);
    } else {
      parentOrientation = orientations[parentInd];
      parentPosition = positions[parentInd];
    }
    orientations[curJoint] = parentOrientation * jointRotations[curJoint];
    positions[curJoint] =
        parentPosition + parentOrientation * skeleton->jointOffset[curJoint];
    if (parentInd == -1)
      positions[curJoint] = rootLocalPosition;
  }
  return positions;
}

vector<vec3> Pose::GetGlobalPositions() {
  vector<quat> orientations;
  return GetGlobalPositionOrientation(orientations);
}

Pose Motion::At(float frame) {
  if (frame <= 0.0f)
    return poses[0];
  if (frame >= poses.size() - 1)
    return poses[poses.size() - 1];
  unsigned int start = (unsigned int)frame;
  unsigned int end = start + 1;
  float alpha = frame - start;
  Pose result;
  result.skeleton = &skeleton;
  result.jointRotations =
      vector<quat>(skeleton.GetNumJoints(), quat(1.0f, vec3(0.0f)));
  result.rootLocalPosition = poses[start].rootLocalPosition * (1.0f - alpha) +
                             poses[end].rootLocalPosition * alpha;
  for (auto jointInd = 0; jointInd < skeleton.GetNumJoints(); ++jointInd) {
    result.jointRotations[jointInd] =
        glm::slerp(poses[start].jointRotations[jointInd],
                   poses[end].jointRotations[jointInd], alpha);
  }
  return result;
}

Pose Skeleton::GetRestPose() {
  Pose p;
  p.skeleton = this;
  p.rootLocalPosition = jointOffset[0];
  int jointNum = GetNumJoints();
  p.jointRotations.resize(jointNum, quat(1.0f, vec3(0.0f)));
  for (int jointInd = 0; jointInd < jointNum; ++jointInd) {
    p.jointRotations[jointInd] = jointRotation[jointInd];
  }
  return p;
}

void Skeleton::ExportAsBVH(std::string filepath, bool keepJointNames) {
  int jointNum = GetNumJoints();
  Animation::Pose emptyPose = GetRestPose();
  emptyPose.skeleton = this;
  Animation::Motion tmpMotion;
  tmpMotion.skeleton = *this;
  tmpMotion.fps = 30;
  tmpMotion.poses.push_back(emptyPose);
  tmpMotion.SaveToBVH(filepath, keepJointNames);
}

vec3 Pose::GetFacingDirection(vec3 restFacing) {
  auto q = jointRotations[0];
  auto xzRot = q * vec3(0.0f, 1.0f, 0.0f);
  auto qxz = Math::FromToRotation(vec3(0.0f, 1.0f, 0.0f), xzRot);
  auto qy = glm::inverse(qxz) * q;
  return qy * restFacing;
}

}; // namespace Animation

}; // namespace aEngine
