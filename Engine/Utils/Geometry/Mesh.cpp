#include "Utils/Geometry/Mesh.hpp"

namespace Geometry {

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

const bool HalfedgeMesh::RemoveVertex(int v) {
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

const bool HalfedgeMesh::RemoveFace(int f) {
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

const int HalfedgeMesh::AddFace(int v1, int v2, int v3) {
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

void HalfedgeMesh::ConstructHalfedges() {
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

}; // namespace Geometry