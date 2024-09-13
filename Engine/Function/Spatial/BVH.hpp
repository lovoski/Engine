#pragma once

#include "Function/Spatial/Types.hpp"

namespace aEngine {

namespace Spatial {

struct Node {
  bool leaf = false;
  // if this node is left child of its parent
  bool left = false;
  int id = -1, lchild = -1, rchild = -1, parent = -1;
  AABB bbox;
  // holding indices to the actual triangle primitives
  std::vector<int> primitives = std::vector<int>();
};

// Use axis-aligned bounding box as bounding volumn,
// triangles as basic primitives.
class BVH {
public:
  BVH();
  ~BVH();

  // Setup primitives data, build the bvh top down.
  void SetAllPrimitives(std::vector<Triangle> &tris);

  // Build bvh top to down with indices to primitives,
  // returns the index for created node
  void BuildTopDown();

  // Check for triangle-triangle intersection with another bvh
  bool Intersect(BVH &other);
  // Check for ray-triangle intersection, setup the first hit position
  bool Intersect(Ray &ray, glm::vec3 &hit);

  const std::vector<Node> &Nodes() { return nodes; }

  int GetNumNodes() { return nodes.size(); }

  // The maximum number of primitives in a leaf node
  int MaxLeafSize = 8;
  // The maximum depth a bvh can gets
  int MaxTreeDepth = 10;

  std::vector<Triangle> Primitives;

private:
  std::vector<Node> nodes;
  std::vector<int> leafNodes;

  AABB computeAABB(std::vector<int> &ids);
  void partitionPrimitives(std::vector<int> &ids, std::vector<int> &leftSubset,
                           std::vector<int> &rightSubset, AABB &bbox);
};

}; // namespace Spatial

}; // namespace aEngine