/**
 * This component holds buffer object of the renderable data,
 * spatial datastructures constructed from the mesh data.
 * 
 * The meshInstance is nullable, be careful with data processing.
 */
#pragma once

#include "Global.hpp"

#include "Base/BaseComponent.hpp"

#include "Function/Render/Mesh.hpp"
#include "Function/Render/Buffers.hpp"

namespace aEngine {

struct Mesh : public BaseComponent {
  Mesh(EntityID id, Render::Mesh *mesh);

  void DrawInspectorGUI() override;

  // Bind vao, ready for rendering
  void Bind();
  // Unbind vao, finish rendering
  void Unbind();

  void SetupMesh(Render::Mesh *mesh);

  // build spatial data structure from renderable mesh
  void BuildSpatialDS(Render::Mesh *mesh);

  // VAO for mesh rendering
  Render::VAO vao;
  // VBO for rendering
  Render::Buffer target;
  // Mark whether the mesh is deformed
  bool Deformed = false;

  Render::Mesh *meshInstance = nullptr;
};

}; // namespace aEngine