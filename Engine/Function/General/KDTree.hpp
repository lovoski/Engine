/**
 * A simple implementation to kdtree, time complexity for a kdtree search is
 * O(n^((k-1)/k)), time complexity for buidling a tree is O(nlogn).
 *
 * kdtree is most useful when the scale of data >> the dimension of data, for
 * high dimensional data search, it might not be as useful.
 */
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <numeric>
#include <stack>
#include <vector>

#include "Function/General/ComputeShader.hpp"
#include "Function/Render/Buffers.hpp"

namespace aEngine {

template <typename Type, std::size_t Dim> class KDTree {
public:
  KDTree() {}
  ~KDTree() {}

  // Build the data structure
  void Build(std::vector<std::array<Type, Dim>> &Data) {
    data = Data;
    std::stack<StackEntry> s;
    std::vector<int> ids(data.size());
    std::iota(ids.begin(), ids.end(), 0);
    // root node
    s.push({true, -1, ids});
    while (!s.empty()) {
      auto entry = s.top();
      s.pop();
      Node node;
      int nodeId = nodes.size();
      node.ids = entry.ids;
      node.parent = entry.parent;
      if (entry.parent != -1) {
        // update linking index
        if (entry.isleft)
          nodes[entry.parent].left = nodeId;
        else
          nodes[entry.parent].right = nodeId;
        // split bounding box based on its parent
        node.bbMin = nodes[entry.parent].bbMin;
        node.bbMax = nodes[entry.parent].bbMax;
        auto &parentPoint = data[nodes[entry.parent].self];
        auto &parentSplitDim = nodes[entry.parent].splitDim;
        if (entry.isleft) {
          node.bbMin[parentSplitDim] =
              std::min(node.bbMin[parentSplitDim], parentPoint[parentSplitDim]);
          node.bbMax[parentSplitDim] =
              std::min(node.bbMax[parentSplitDim], parentPoint[parentSplitDim]);
        } else {
          node.bbMin[parentSplitDim] =
              std::max(node.bbMin[parentSplitDim], parentPoint[parentSplitDim]);
          node.bbMax[parentSplitDim] =
              std::max(node.bbMax[parentSplitDim], parentPoint[parentSplitDim]);
        }
      } else {
        node.bbMin.fill(std::numeric_limits<Type>::max());
        node.bbMax.fill(-std::numeric_limits<Type>::max());
        for (auto &a : data) {
          for (int i = 0; i < Dim; ++i) {
            node.bbMin[i] = std::min(node.bbMin[i], a[i]);
            node.bbMax[i] = std::max(node.bbMax[i], a[i]);
          }
        }
      }
      if (entry.ids.size() > 1) {
        // select split dimension
        int splitDim = entry.parent == -1 ? 0 : (entry.parent + 1) % Dim;
        node.splitDim = splitDim;
        // select split point, O(n)
        int midIndex = entry.ids.size() / 2;
        std::nth_element(entry.ids.begin(), entry.ids.begin() + midIndex,
                         entry.ids.end(), [&](int i1, int i2) {
                           return data[i1][splitDim] < data[i2][splitDim];
                         });
        Type splitVal = data[entry.ids[midIndex]][splitDim];
        node.self = entry.ids[midIndex]; // index for split data
        std::vector<int> leftSplit, rightSplit;
        for (int i = 0; i < entry.ids.size(); ++i) {
          int id = entry.ids[i];
          if (data[id][splitDim] < splitVal) {
            leftSplit.push_back(entry.ids[i]);
          } else if (data[id][splitDim] > splitVal) {
            rightSplit.push_back(entry.ids[i]);
          } else {
            // push to left if not the split point
            if (entry.ids[i] != node.self)
              leftSplit.push_back(entry.ids[i]);
          }
        }
        if (rightSplit.size() > 0)
          s.push({false, (int)nodes.size(), rightSplit});
        if (leftSplit.size() > 0)
          s.push({true, (int)nodes.size(), leftSplit});
      } else {
        node.self = entry.ids[0];
      }
      nodes.emplace_back(node);
    }

    // setup variable for search
    kdsearchs.resize(nodes.size() + 1, -1);

    char shaderBuffer[1024];
    snprintf(shaderBuffer, sizeof(shaderBuffer), parelleSearchSource.c_str(),
             Dim, Dim);
    cs = std::make_shared<ComputeShader>(shaderBuffer);
    _inputData.resize(data.size() + 1);
    for (int i = 0; i < Dim; ++i) {
      for (int j = 1; j <= data.size(); ++j)
        _inputData[j].pos[i] = data[j - 1][i];
    }
    _outputData.resize(data.size(), 0.0f);
    _output.SetDataAs(GL_SHADER_STORAGE_BUFFER, _outputData);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  }

  // Nearest neighbor search with kdtree, returns index for the closest data
  int NearestSearch(std::array<Type, Dim> &target) {
    Type nearestVal = std::numeric_limits<Type>::max();
    int nearestInd = -1, stackTop = 0;
    kdsearchs[0] = 0;
    stackTop++;
    while (stackTop != 0) {
      int cur = kdsearchs[--stackTop];
      if (cur == -1)
        continue;
      const Node &curNode = nodes[cur];
      int splitDim = nodes[cur].splitDim;
      Type dist = euclideanDist(data[curNode.self], target);
      if (nearestVal > dist) {
        nearestVal = dist;
        nearestInd = curNode.self;
      }
      if (splitDim != -1) {
        int nextBranch, otherBranch;
        // equal to current split in left subtree
        if (target[splitDim] <= data[curNode.self][splitDim]) {
          // more likely nearest to left subtree, search left first
          nextBranch = curNode.left;
          otherBranch = curNode.right;
        } else {
          nextBranch = curNode.right;
          otherBranch = curNode.left;
        }
        kdsearchs[stackTop++] = nextBranch;
        // don't search the other branch when its already too far from current
        // node
        if (std::abs(target[splitDim] - data[curNode.self][splitDim]) <=
            nearestVal)
          kdsearchs[stackTop++] = otherBranch;
      }
    }
    return nearestInd;
  }

  // Nearest search with brute force
  int BruteForceNearestSearch(std::array<Type, Dim> &target) {
    Type nearestVal = std::numeric_limits<Type>::max();
    int nearestInd = -1;
    for (int i = 0; i < data.size(); ++i) {
      auto val = euclideanDist(target, data[i]);
      if (val < nearestVal) {
        nearestInd = i;
        nearestVal = val;
      }
    }
    return nearestInd;
  }

  int ParelleNearestSearch(std::array<Type, Dim> &target) {
    cs->Use();
    for (int i = 0; i < Dim; ++i)
      _inputData[0].pos[i] = target[i];
    _input.SetDataAs(GL_SHADER_STORAGE_BUFFER, _inputData);
    _input.BindToPointAs(GL_SHADER_STORAGE_BUFFER, 0);
    _output.BindToPointAs(GL_SHADER_STORAGE_BUFFER, 1);
    cs->Dispatch((data.size() + 63) / 64, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    // read from gpu
    _output.BindAs(GL_SHADER_STORAGE_BUFFER);
    float *mappedData =
        (float *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
                                  sizeof(float) * data.size(), GL_MAP_READ_BIT);
    int nearestInd = -1;
    float nearestVal = std::numeric_limits<float>::max();
    if (mappedData) {
      // find the smallest distance
      for (int i = 0; i < data.size(); ++i) {
        if (mappedData[i] < nearestVal) {
          nearestInd = i;
          nearestVal = mappedData[i];
        }
      }
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    return nearestInd;
  }

  // Get data with a index
  const std::array<Type, Dim> &Data(int index) const { return data[index]; }

private:
  std::vector<std::array<Type, Dim>> data;

  struct _Point {
    float pos[Dim];
  };
  Render::Buffer _input, _output;
  std::vector<_Point> _inputData;
  std::vector<float> _outputData;
  std::shared_ptr<ComputeShader> cs;
  std::string parelleSearchSource = R"(
  #version 430 core
  layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
  struct Point {
    float pos[%d];
  };
  layout(std430, binding = 0) buffer Input {
    Point p[];
  };
  layout(std430, binding = 1) buffer Output {
    float dist[];
  };
  void main() {
    uint index = gl_GlobalInvocationID.x + 1;
    Point target = p[0];
    Point source = p[index];
    float result = 0;
    for (int i = 0; i < %d; ++i)
      result += (target.pos[i]-source.pos[i])*(target.pos[i]-source.pos[i]);
    dist[index-1] = result;
  }
  )";

  struct StackEntry {
    bool isleft;
    int parent;
    std::vector<int> ids;
  };
  struct Node {
    int splitDim = -1, self = -1;
    int left = -1, right = -1, parent = -1;
    std::vector<int> ids;
    // bounding box containing the substree
    std::array<Type, Dim> bbMin, bbMax;
  };
  std::vector<Node> nodes;

  Type euclideanDist(std::array<Type, Dim> &a, std::array<Type, Dim> &b) {
    Type result = 0;
    for (int i = 0; i < Dim; ++i)
      result += (a[i] - b[i]) * (a[i] - b[i]);
    return std::sqrt(result);
  }

  std::vector<int> kdsearchs;
};

using KDTree2f = KDTree<float, 2>;
using KDTree3f = KDTree<float, 3>;

}; // namespace aEngine