#pragma once

#include "Function/Spatial/Types.hpp"

namespace aEngine {

namespace Spatial {

struct Node {
  bool leaf = false;
  int id = -1, lchild = -1, rchild = -1;
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

  void Clear() {
    nodes.clear();
    primitives.clear();
  }

  // Setup primitives data, build the bvh top down.
  void SetAllPrimitives(std::vector<Triangle> &tris);

  // Build bvh top to down with indices to primitives,
  // returns the index for created node
  int BuildTopDown(std::vector<int> &ids);

  // Check for triangle-triangle intersection with another bvh
  bool Intersect(BVH &other);
  // Check for ray-triangle intersection, setup the first hit position
  bool Intersect(Ray &ray, glm::vec3 &hit);

  Triangle &Primitive(int id) { return primitives[id]; }
  const std::vector<Node> &Nodes() { return nodes; }

  int GetNumNodes() { return nodes.size(); }
  int GetNumPrimitives() { return primitives.size(); }

  // The maximum number of primitives in a leaf node
  int MaxLeafSize = 8;

private:
  std::vector<Node> nodes;
  std::vector<Triangle> primitives;

  AABB computeAABB(std::vector<int> &ids);
  void partitionPrimitives(std::vector<int> &ids, std::vector<int> &leftSubset,
                           std::vector<int> &rightSubset, AABB &bbox);
};

}; // namespace Spatial

}; // namespace aEngine