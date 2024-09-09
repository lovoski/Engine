/**
 * This component holds buffer object of the renderable data,
 * spatial datastructures constructed from the mesh data.
 *
 * The meshInstance is never nullptr. If the parameter to constructor
 * is nullptr, load cube primitive by default.
 */
#pragma once

#include "Global.hpp"

#include "Base/BaseComponent.hpp"

#include "Function/Render/Buffers.hpp"
#include "Function/Render/Mesh.hpp"


namespace aEngine {

struct Mesh : public BaseComponent {
  Mesh(EntityID id, Render::Mesh *mesh);

  void DrawInspectorGUI() override;

  // Bind vao, ready for rendering
  void Bind();
  // Unbind vao, finish rendering
  void Unbind();

  // build spatial data structure from renderable mesh
  void BuildSpatialDS(Render::Mesh *mesh);

  void SetMeshInstance(Render::Mesh *mesh);
  // The mesh instance is not allowed to be nullptr,
  // this function should be safe to use.
  Render::Mesh *GetMeshInstance() { return meshInstance; }

  // VAO for mesh rendering
  Render::VAO vao;
  // VBO for rendering
  Render::Buffer target;
  // Mark whether the mesh is deformed
  bool Deformed = false;

private:
  Render::Mesh *meshInstance = nullptr;
};

}; // namespace aEngine