#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

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
                 std::vector<TriFace> &faces) {
  // Read the OFF file
  std::string line, tmp;
  std::ifstream offfile(filename);
  double a1, a2, a3;
  int numFaces, numVertices;
  if (offfile.is_open()) {
    // Check first line is a OFF file
    while (std::getline(offfile, line)) { // add check boundary vertices flag
      std::istringstream(line) >> tmp;
      if (tmp[0] != '#' && !isWhitespace(line)) {
        if (tmp[0] == 'O' && tmp[1] == 'F' &&
            tmp[2] == 'F') // Check if the format is OFF
          break;
        else {
          std::cout << "The file is not an OFF file" << std::endl;
          exit(0);
        }
      }
    }
    // Read the number of vertices and faces
    while (std::getline(offfile, line)) { // add check boundary vertices flag
      std::istringstream(line) >> tmp;
      if (tmp[0] != '#' && !isWhitespace(line)) {
        std::istringstream(line) >> numVertices >> numFaces;
        // Create space for internal storage
        vertices.resize(numVertices);
        faces.resize(numFaces);
        break;
      }
    }
    // Read vertices
    int index = 0;
    while (index < numVertices && std::getline(offfile, line)) {
      std::istringstream(line) >> tmp;
      if (tmp[0] != '#' && !isWhitespace(line)) {
        std::istringstream(line) >> a1 >> a2 >> a3;
        Vertex v;
        v.id = index;
        v.x = a1;
        v.y = a2;
        v.z = a3;
        vertices[index] = v;
        index++;
      }
    }
    // Read faces
    int verticesPerFace, t1, t2, t3;
    index = 0;
    while (index < numFaces && std::getline(offfile, line)) {
      std::istringstream(line) >> tmp;
      if (tmp[0] != '#' && !isWhitespace(line)) {
        std::istringstream(line) >> verticesPerFace >> t1 >> t2 >> t3;
        TriFace face;
        face.id = index;
        face.v[0] = t1;
        face.v[1] = t2;
        face.v[2] = t3;
        faces[index] = face;
        index++;
      }
    }
  } else
    std::cout << "Unable to open node file";
  offfile.close();
}

void ExportToOff(std::string filename, std::vector<Vertex> &vertices,
                 std::vector<TriFace> &faces) {
  std::ofstream offFile(filename);
  if (offFile.is_open()) {
    offFile << "OFF" << std::endl;
    offFile << vertices.size() << " " << faces.size() << " 0" << std::endl;
    for (const Vertex &vertex : vertices) {
      offFile << vertex.x << " " << vertex.y << " " << vertex.z << std::endl;
    }
    for (const TriFace &face : faces) {
      offFile << "3 " << face.v[0] << " " << face.v[1] << " " << face.v[2]
              << std::endl;
    }
    offFile.close();
  } else {
    std::cout << "Unable to create or open the file: " << filename << std::endl;
  }
}

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
  const bool RemoveVertex(int v) {
    if (!VertexInRange(v)) {
      printf("vertex index out of range\n");
      return false;
    }

    std::vector<int> halfedgesToRemove;
    std::vector<int> facesToRemove;

    // Collect all halfedges and faces connected to this vertex
    int startEdge = vertices[v].edge;
    int edge = startEdge;
    do {
      halfedgesToRemove.push_back(edge);
      facesToRemove.push_back(halfedges[edge].face);
      edge = halfedges[halfedges[edge].twin].next;
    } while (edge != startEdge);

    // Remove the faces
    for (int face : facesToRemove) {
      if (!RemoveFace(face)) {
        printf("Error removing face %d\n", face);
        return false;
      }
    }

    // Remove the halfedges
    for (int he : halfedgesToRemove) {
      halfedges[he].origin = -1;
      halfedges[he].target = -1;
      halfedges[he].face = -1;
      halfedges[he].twin = -1;
      halfedges[he].next = -1;
      halfedges[he].prev = -1;
    }

    // Remove the vertex
    vertices[v].x = vertices[v].y = vertices[v].z = 0.0;
    vertices[v].edge = -1;
    vertices[v].border = false;
    vertices[v].id = -1;
    numVertices--;

    return true;
  }
  // By removing the face, only the three halfedges are removed
  // By removing the face, only the three halfedges are removed
  const bool RemoveFace(int f) {
    if (!FaceInRange(f)) {
      printf("face index out of range\n");
      return false;
    }

    // Collect the halfedges of the face
    int he1 = 3 * f;
    int he2 = he1 + 1;
    int he3 = he1 + 2;

    // Invalidate the face
    faces[f].id = -1;
    faces[f].v[0] = faces[f].v[1] = faces[f].v[2] = -1;
    numFaces--;

    // Invalidate the halfedges
    halfedges[he1].face = -1;
    halfedges[he2].face = -1;
    halfedges[he3].face = -1;
    halfedges[he1].origin = -1;
    halfedges[he2].origin = -1;
    halfedges[he3].origin = -1;
    halfedges[he1].target = -1;
    halfedges[he2].target = -1;
    halfedges[he3].target = -1;

    // Update twin halfedges to mark them as boundary if needed
    if (halfedges[he1].twin != -1) {
      halfedges[halfedges[he1].twin].twin = -1;
      halfedges[halfedges[he1].twin].face = -1;
    }
    if (halfedges[he2].twin != -1) {
      halfedges[halfedges[he2].twin].twin = -1;
      halfedges[halfedges[he2].twin].face = -1;
    }
    if (halfedges[he3].twin != -1) {
      halfedges[halfedges[he3].twin].twin = -1;
      halfedges[halfedges[he3].twin].face = -1;
    }

    return true;
  }

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
  const int AddFace(int v1, int v2, int v3) {
    CHECK_VERT_IN_RANGE(v1);
    CHECK_VERT_IN_RANGE(v2);
    CHECK_VERT_IN_RANGE(v3);

    // Check for existing face
    if (ExistFace(v1, v2, v3)) {
      printf("The face already exists\n");
      return -1;
    }

    // Create a new face
    TriFace newFace;
    newFace.id = faces.size();
    newFace.v[0] = v1;
    newFace.v[1] = v2;
    newFace.v[2] = v3;
    faces.push_back(newFace);
    numFaces++;

    // Create halfedges for the new face
    int newHalfedges[3];
    for (int i = 0; i < 3; i++) {
      Halfedge he;
      he.id = halfedges.size();
      he.face = newFace.id;
      he.origin = newFace.v[i];
      he.target = newFace.v[(i + 1) % 3];
      he.prev = newHalfedges[(i + 2) % 3]; // Set later
      he.next = newHalfedges[(i + 1) % 3]; // Set later
      he.twin = -1;                        // Set later
      halfedges.push_back(he);
      newHalfedges[i] = he.id;
    }

    // Set prev and next for the new halfedges
    halfedges[newHalfedges[0]].prev = newHalfedges[2];
    halfedges[newHalfedges[0]].next = newHalfedges[1];
    halfedges[newHalfedges[1]].prev = newHalfedges[0];
    halfedges[newHalfedges[1]].next = newHalfedges[2];
    halfedges[newHalfedges[2]].prev = newHalfedges[1];
    halfedges[newHalfedges[2]].next = newHalfedges[0];

    // Update vertex properties
    for (int i = 0; i < 3; i++) {
      int vert = newFace.v[i];
      if (vertices[vert].edge == -1 || vertices[vert].border) {
        vertices[vert].edge = newHalfedges[i];
        vertices[vert].border = false;
        numBorderVertices--;
      }
    }

    // Establish twin relationships with existing halfedges
    for (int i = 0; i < 3; i++) {
      int vOrigin = newFace.v[i];
      int vTarget = newFace.v[(i + 1) % 3];
      bool twinFound = false;
      for (int j = 0; j < halfedges.size(); j++) {
        if (halfedges[j].origin == vTarget && halfedges[j].target == vOrigin) {
          halfedges[newHalfedges[i]].twin = halfedges[j].id;
          halfedges[j].twin = newHalfedges[i];
          twinFound = true;
          break;
        }
      }
      if (!twinFound) {
        // Create a boundary halfedge
        Halfedge boundaryHe;
        boundaryHe.id = halfedges.size();
        boundaryHe.face = -1;
        boundaryHe.origin = vTarget;
        boundaryHe.target = vOrigin;
        boundaryHe.twin = newHalfedges[i];
        boundaryHe.prev = -1; // Set later
        boundaryHe.next = -1; // Set later
        halfedges.push_back(boundaryHe);

        // Update twin relationship
        halfedges[newHalfedges[i]].twin = boundaryHe.id;
      }
    }

    // Update border vertices and halfedges
    for (int i = 0; i < 3; i++) {
      int borderEdge = halfedges[newHalfedges[i]].twin;
      if (borderEdge != -1 && halfedges[borderEdge].face == -1) {
        int nextBorderEdge = halfedges[borderEdge].next;
        while (nextBorderEdge != -1 && halfedges[nextBorderEdge].face != -1) {
          nextBorderEdge = halfedges[halfedges[nextBorderEdge].twin].next;
        }
        halfedges[borderEdge].next = nextBorderEdge;

        int prevBorderEdge = halfedges[borderEdge].prev;
        while (prevBorderEdge != -1 && halfedges[prevBorderEdge].face != -1) {
          prevBorderEdge = halfedges[halfedges[prevBorderEdge].twin].prev;
        }
        halfedges[borderEdge].prev = prevBorderEdge;
      }
    }

    return newFace.id;
  }

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
  void ConstructHalfedges() {
    std::vector<std::vector<std::pair<int, int>>> map(numVertices);
    for (int i = 0; i < numFaces; ++i) {
      for (int j = 0; j < 3; ++j) {
        Halfedge he;
        he.id = i * 3 + j;
        int vOrigin = faces[i].v[j];
        int vTarget = faces[i].v[(j + 1) % 3];
        int minVertId = std::min(vOrigin, vTarget);
        int maxVertId = std::max(vOrigin, vTarget);
        he.twin = -1;
        if (map[minVertId].size() == 0) {
          // No twin has been stored
          map[minVertId].push_back(std::make_pair(maxVertId, i * 3 + j));
        } else {
          // Query the map for its twin with the same maxVertId
          for (auto stores : map[minVertId]) {
            if (stores.first == maxVertId) {
              // Twin found
              he.twin = stores.second;
              halfedges[stores.second].twin = i * 3 + j;
            }
          }
          if (he.twin == -1) {
            // Still found no twin
            map[minVertId].push_back(std::make_pair(maxVertId, i * 3 + j));
          }
        }
        he.origin = vOrigin;
        he.target = vTarget;
        he.prev = i * 3 + (j + 2) % 3;
        he.next = i * 3 + (j + 1) % 3;
        he.face = i;
        halfedges[i * 3 + j] = he;
        vertices[vOrigin].edge = i * 3 + j;
      }
    }
    // Locate vertices on border
    numBorderVertices = 0;
    std::vector<int> borderEdgeIds;
    for (int i = 0; i < numHalfedges; ++i) {
      if (halfedges[i].twin == -1) {
        numBorderVertices++;
        borderEdgeIds.push_back(i);
        vertices[halfedges[i].origin].border = true;
        vertices[halfedges[i].target].border = true;
      }
    }
    if (numBorderVertices > 0) {
      // Extend halfedge array to exterior edges
      halfedges.reserve(numHalfedges + numBorderVertices);
      for (int i = numHalfedges; i < numHalfedges + numBorderVertices; ++i) {
        Halfedge he;
        he.id = i;
        he.face = -1;
        he.twin = borderEdgeIds[i - numHalfedges];
        halfedges[he.twin].twin = i;
        he.origin = halfedges[he.twin].target;
        he.target = halfedges[he.twin].origin;
        halfedges.push_back(he);
      }
      // Build exterior edges prev and next
      for (int i = numHalfedges; i < numHalfedges + numBorderVertices; ++i) {
        int cur = halfedges[i].twin;
        while (halfedges[cur].face != -1)
          cur = halfedges[halfedges[cur].next].twin;
        halfedges[i].prev = cur;
        cur = halfedges[i].twin;
        while (halfedges[cur].face != -1)
          cur = halfedges[halfedges[cur].prev].twin;
        halfedges[i].next = cur;
      }
      numHalfedges = numHalfedges + numBorderVertices;
    }
  }
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