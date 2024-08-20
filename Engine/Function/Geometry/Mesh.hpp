#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cmath>

namespace Geometry {

#define CHECK_EDGE_IN_RANGE(e)                                                 \
  if (!EdgeInRange(e)) {                                                       \
    printf("edge index out of range\n");                                       \
    return -1;                                                                 \
  }
#define CHECK_VERT_IN_RANGE(v)                                                 \
  if (!VertexInRange(v)) {                                                     \
    printf("vertex index out of range\n");                                     \
    return -1;                                                                 \
  }
#define CHECK_FACE_IN_RANGE(f)                                                 \
  if (!FaceInRange(f)) {                                                       \
    printf("face index out of range\n");                                       \
    return -1;                                                                 \
  }

struct TriFace;
struct Vertex;
struct Halfedge;

struct Vertex {
  int id;
  double x, y, z;
  int edge; // one half edge take current vertex as origin
  bool border = false;
};

struct Halfedge {
  int id;
  int face;
  int origin, target;
  int twin, next, prev;
};

struct TriFace {
  int id;
  int v[3]; // indices for the triangle vertices
};

// Returns false if the string contains any non-whitespace characters
// Returns false if the string contains any non-ASCII characters
bool isWhitespace(std::string s) {
  for (int index = 0; index < s.length(); index++)
    if (!std::isspace(s[index]))
      return false;
  return true;
}

void LoadFromOff(std::string filename, std::vector<Vertex> &vertices,
                 std::vector<TriFace> &faces);

void ExportToOff(std::string filename, std::vector<Vertex> &vertices,
                 std::vector<TriFace> &faces);

// Halfedge mesh data structure for manifold geometry
class HalfedgeMesh {
public:
  HalfedgeMesh(std::vector<Vertex> &v, std::vector<TriFace> &f) {
    faces = f;
    numFaces = f.size();
    vertices = v;
    numVertices = v.size();
    halfedges.resize(numFaces * 3);
    numHalfedges = numFaces * 3;
    ConstructHalfedges();
  }
  ~HalfedgeMesh() {}

  const int GetNumHalfedges() { return numHalfedges; }
  const int GetNumVertices() { return numVertices; }
  const int GetNumFaces() { return numFaces; }

  // Query

  const int Next(int e) {
    CHECK_EDGE_IN_RANGE(e)
    return halfedges[e].next;
  }
  const int Prev(int e) {
    CHECK_EDGE_IN_RANGE(e)
    return halfedges[e].prev;
  }
  const int Twin(int e) {
    CHECK_EDGE_IN_RANGE(e)
    return halfedges[e].twin;
  }
  const int Face(int e) {
    CHECK_EDGE_IN_RANGE(e)
    return halfedges[e].face;
  }
  const int Origin(int e) {
    CHECK_EDGE_IN_RANGE(e)
    return halfedges[e].origin;
  }
  const int Target(int e) {
    CHECK_EDGE_IN_RANGE(e)
    return halfedges[e].target;
  }
  const int EdgeOfVertex(int v) {
    CHECK_VERT_IN_RANGE(v)
    return vertices[v].edge;
  }
  const std::vector<int> NeighborsOfVertex(int v) {
    std::vector<int> ret;
    if (!VertexInRange(v)) {
      printf("vertex index out of range\n");
      return ret;
    }
    int eid = vertices[v].edge;
    if (eid == -1) {
      printf("vertex has no neighbor\n");
      return ret;
    }
    int cur = eid;
    do {
      ret.push_back(halfedges[cur].target);
      cur = halfedges[cur].prev;
      cur = halfedges[cur].twin;
    } while (cur != eid);
    return ret;
  }

  // The border vertices can be recognized by the border property
  const bool IsBorderVertex(int v) {
    if (!VertexInRange(v)) {
      printf("vertex index out of range\n");
      return false;
    }
    return vertices[v].border;
  }

  // The halfedges is border if the faceid is -1
  const bool IsBorderHalfedge(int e) {
    if (!EdgeInRange(e)) {
      printf("edge index out of range\n");
      return false;
    }
    return halfedges[e].face == -1;
  }

  // Modification

  // By removing the vertex, the halfedges and faces connected
  // will also be removed
  const bool RemoveVertex(int v);

  // By removing the face, only the three halfedges are removed
  // By removing the face, only the three halfedges are removed
  const bool RemoveFace(int f);

  // Push one element to the vertices array, no connection will be made
  // The index for created vertex will be returned
  const int AddVertex(double x, double y, double z) {
    Vertex newVert;
    newVert.id = vertices.size();
    newVert.x = x;
    newVert.y = y;
    newVert.z = z;
    newVert.border = true; // mark new vertices on border
    newVert.edge = -1;     // no connectivity with other vertices
    vertices.push_back(newVert);
    numVertices++;
    numBorderVertices++;
    return newVert.id;
  }

  // Create face connection between three given vertices
  // The index for created face will be returned
  // The operation would be abort if it makes the model non-manifold
  // The normal for the new face would be cross((v2-v1),(v3-v2))
  const int AddFace(int v1, int v2, int v3);

private:
  int numHalfedges, numVertices, numFaces;
  int numBorderVertices;
  std::vector<Vertex> vertices;
  std::vector<TriFace> faces;
  std::vector<Halfedge> halfedges;

  bool VertexInRange(int v) { return v >= 0 && v < numVertices; }
  bool EdgeInRange(int e) { return e >= 0 && e < numHalfedges; }
  bool FaceInRange(int f) { return f >= 0 && f < numFaces; }

  // Assume that the three vertices are in range
  bool ExistFace(int v1, int v2, int v3) {
    auto neigh = NeighborsOfVertex(v1);
    auto neighSize = neigh.size();
    for (int i = 0; i < neighSize; ++i) {
      if (neigh[i] == v2 && i + 1 < neighSize && neigh[i + 1] == v3)
        return true;
      if (neigh[i] == v3 && i + 1 < neighSize && neigh[i + 1] == v2)
        return true;
    }
    return false;
  }

  // Convert each triangle face into three halfedges
  // Traversal the edges and create exteroir edges
  void ConstructHalfedges();
};

class Icosphere {
public:
  Icosphere(int subdivisions) {
    // Create the initial icosahedron
    createIcosahedron();
    // Subdivide faces
    for (int i = 0; i < subdivisions; ++i) {
      subdivide();
    }
    // Normalize vertices to lie on the unit sphere
    normalizeVertices();
  }

  std::vector<Vertex> GetVertices() const { return vertices; }
  std::vector<TriFace> GetFaces() {
    InvertFaces();
    return faces;
  }

  void InvertFaces() {
    for (auto &face : faces) {
      std::swap(face.v[1], face.v[2]);
    }
  }

private:
  std::vector<Vertex> vertices;
  std::vector<TriFace> faces;
  std::unordered_map<uint64_t, int> midPointCache;

  void createIcosahedron() {
    const double X = 0.525731112119133606;
    const double Z = 0.850650808352039932;
    const double N = 0.0;

    vertices = {{0, -X, N, Z}, {1, X, N, Z},  {2, -X, N, -Z}, {3, X, N, -Z},
                {4, N, Z, X},  {5, N, Z, -X}, {6, N, -Z, X},  {7, N, -Z, -X},
                {8, Z, X, N},  {9, -Z, X, N}, {10, Z, -X, N}, {11, -Z, -X, N}};

    faces = {
        {0, {0, 4, 1}},   {1, {0, 9, 4}},   {2, {9, 5, 4}},   {3, {4, 5, 8}},
        {4, {4, 8, 1}},   {5, {8, 10, 1}},  {6, {8, 3, 10}},  {7, {5, 3, 8}},
        {8, {5, 2, 3}},   {9, {2, 7, 3}},   {10, {7, 10, 3}}, {11, {7, 6, 10}},
        {12, {7, 11, 6}}, {13, {11, 0, 6}}, {14, {0, 1, 6}},  {15, {6, 1, 10}},
        {16, {9, 0, 11}}, {17, {9, 11, 2}}, {18, {9, 2, 5}},  {19, {7, 2, 11}}};
  }

  void subdivide() {
    std::vector<TriFace> newFaces;
    int currentId = 0;
    for (const auto &face : faces) {
      int v1 = face.v[0];
      int v2 = face.v[1];
      int v3 = face.v[2];

      int a = getMidPoint(v1, v2);
      int b = getMidPoint(v2, v3);
      int c = getMidPoint(v3, v1);

      newFaces.push_back(TriFace{currentId++, {v1, a, c}});
      newFaces.push_back(TriFace{currentId++, {v2, b, a}});
      newFaces.push_back(TriFace{currentId++, {v3, c, b}});
      newFaces.push_back(TriFace{currentId++, {a, b, c}});
    }
    faces = newFaces;
  }

  int getMidPoint(int v1, int v2) {
    bool firstIsSmaller = v1 < v2;
    uint64_t smallerIndex = firstIsSmaller ? v1 : v2;
    uint64_t greaterIndex = firstIsSmaller ? v2 : v1;
    uint64_t key = (smallerIndex << 32) + greaterIndex;

    auto it = midPointCache.find(key);
    if (it != midPointCache.end()) {
      return it->second;
    }

    Vertex vert1 = vertices[v1];
    Vertex vert2 = vertices[v2];
    Vertex mid = {static_cast<int>(vertices.size()), (vert1.x + vert2.x) / 2.0,
                  (vert1.y + vert2.y) / 2.0, (vert1.z + vert2.z) / 2.0};

    vertices.push_back(mid);
    midPointCache[key] = mid.id;
    return mid.id;
  }

  void normalizeVertices() {
    for (auto &vertex : vertices) {
      double length = std::sqrt(vertex.x * vertex.x + vertex.y * vertex.y +
                                vertex.z * vertex.z);
      vertex.x /= length;
      vertex.y /= length;
      vertex.z /= length;
    }
  }
};

}; // namespace Geometry