#include "Function/Spatial/BVH.hpp"

namespace aEngine {

namespace Spatial {

BVH::BVH() {}

BVH::~BVH() {}

void BVH::SetAllPrimitives(std::vector<Triangle> &tris) {
  Primitives = tris;
  BuildTopDown();
}

struct StackEntry1 {
  int depth;
  std::vector<int> ids;
  int parentNodeId;
  bool isLeftChild;
};
void BVH::BuildTopDown() {
  // prepare data
  nodes.clear();
  leafNodes.clear();
  std::vector<int> ids;
  for (int i = 0; i < Primitives.size(); ++i)
    ids.push_back(i);
  std::stack<StackEntry1> stack;

  // root node
  stack.push({0, ids, -1, false});

  while (!stack.empty()) {
    auto entry = stack.top();
    stack.pop();

    Node node;
    node.id = nodes.size();
    node.parent = entry.parentNodeId;
    node.left = entry.isLeftChild;
    node.bbox = computeAABB(entry.ids);

    // leaf node
    if (entry.ids.size() <= MaxLeafSize || entry.depth >= MaxTreeDepth) {
      node.leaf = true;
      node.primitives = entry.ids;
      nodes.emplace_back(node);
      // keep record of the leaf nodes
      leafNodes.push_back(node.id);
    } else {
      // internal node spliting the primitives
      node.leaf = false;
      nodes.emplace_back(node);

      std::vector<int> leftSubset, rightSubset;
      partitionPrimitives(entry.ids, leftSubset, rightSubset, node.bbox);
      stack.push({entry.depth + 1, rightSubset, node.id, false});
      stack.push({entry.depth + 1, leftSubset, node.id, true});
    }
  }
  // update parent-child relation after all nodes been created
  for (auto &node : nodes) {
    if (node.parent != -1) {
      if (node.left)
        nodes[node.parent].lchild = node.id;
      else
        nodes[node.parent].rchild = node.id;
    }
  }
}

AABB BVH::computeAABB(std::vector<int> &ids) {
  auto min = glm::vec3(std::numeric_limits<float>::max()),
       max = glm::vec3(-std::numeric_limits<float>::max());
  // brute force
  for (auto id : ids) {
    auto &tri = Primitives[id];
    for (int i = 0; i < 3; ++i) {
      // triangle indices
      for (int j = 0; j < 3; ++j) {
        // xyz axis
        min[j] = min[j] < tri.V[i][j] ? min[j] : tri.V[i][j];
        max[j] = max[j] > tri.V[i][j] ? max[j] : tri.V[i][j];
      }
    }
  }
  return {min, max};
}

void BVH::partitionPrimitives(std::vector<int> &ids,
                              std::vector<int> &leftSubset,
                              std::vector<int> &rightSubset, AABB &bbox) {
  if (ids.size() == 0) {
    std::cout << "partioning an array with no primitives" << std::endl;
    return;
  }
  // find an axis to partition
  // use the longest axis in bbox
  auto bboxSize = bbox.Max - bbox.Min;
  glm::vec3 axis;
  if (bboxSize[1] > bboxSize[0]) {
    if (bboxSize[2] > bboxSize[1])
      axis = glm::vec3(0.0f, 0.0f, 1.0f);
    else
      axis = glm::vec3(0.0f, 1.0f, 0.0f);
  } else {
    if (bboxSize[2] > bboxSize[0])
      axis = glm::vec3(0.0f, 0.0f, 1.0f);
    else
      axis = glm::vec3(1.0f, 0.0f, 0.0f);
  }
  // find the position to partition
  // project the barycenter of all triangles to this axis
  // sort and find the middle value to do partition
  std::vector<float> val;
  for (auto id : ids) {
    auto &tri = Primitives[id];
    val.push_back(glm::dot(axis, tri.Barycenter()));
  }
  std::sort(val.begin(), val.end());
  float midVal = val[val.size() / 2];
  for (auto id : ids) {
    auto &tri = Primitives[id];
    if (glm::dot(axis, tri.Barycenter()) < midVal)
      leftSubset.push_back(id);
    else
      rightSubset.push_back(id);
  }
}

bool BVH::Intersect(BVH &other, std::vector<std::pair<int, int>> &hit) {
  hit.clear();
  // stores the pair of nodes to process
  std::stack<std::pair<int, int>> s;
  s.push(std::make_pair(0, 0)); // start from root node
  while (!s.empty()) {
    auto entry = s.top();
    s.pop();
    if (entry.first == -1 || entry.second == -1)
      continue;
    auto &first = nodes[entry.first];
    auto &second = other.nodes[entry.second];
    if (first.bbox.Test(second.bbox)) {
      if (first.leaf && second.leaf) {
        // both leaf nodes, perform triangle tests
        for (auto firstId : first.primitives)
          for (auto secondId : second.primitives)
            if (Primitives[firstId].Test(other.Primitives[secondId]))
              hit.push_back(std::make_pair(firstId, secondId));
      } else if (first.leaf && !second.leaf) {
        s.push(std::make_pair(entry.first, second.lchild));
        s.push(std::make_pair(entry.first, second.rchild));
      } else if (!first.leaf && second.leaf) {
        s.push(std::make_pair(first.lchild, entry.second));
        s.push(std::make_pair(first.rchild, entry.second));
      } else {
        // both nodes are not leaf node, check all children pairs
        s.push(std::make_pair(first.lchild, second.lchild));
        s.push(std::make_pair(first.lchild, second.rchild));
        s.push(std::make_pair(first.rchild, second.lchild));
        s.push(std::make_pair(first.rchild, second.rchild));
      }
    }
  }
  return hit.size() > 0;
}

bool BVH::Intersect(Ray &ray, glm::vec3 &hit) { return false; }

}; // namespace Spatial

}; // namespace aEngine