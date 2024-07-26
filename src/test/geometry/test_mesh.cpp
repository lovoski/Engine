#include "Mesh.hpp"

int main() {
  int subdivisions = 2; // Adjust the number of subdivisions as needed
  Geometry::Icosphere icosphere(subdivisions);

  std::vector<Geometry::Vertex> vertices = icosphere.GetVertices();
  std::vector<Geometry::TriFace> faces = icosphere.GetFaces();

  // Export the icosphere to an OFF file
  Geometry::ExportToOff("icosphere.off", vertices, faces);

  return 0;
}