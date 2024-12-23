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

#include "Function/Spatial/Types.hpp"
#include "Function/Spatial/BVH.hpp"

namespace aEngine {

struct Mesh : public BaseComponent {
  Mesh() : BaseComponent(0) {}
  Mesh(EntityID id, Render::Mesh *mesh);

  void DrawInspectorGUI() override;

  // Bind vao, ready for rendering
  void Bind();
  // Unbind vao, finish rendering
  void Unbind();

  void SetMeshInstance(Render::Mesh *mesh);
  // The mesh instance is not allowed to be nullptr,
  // this function should be safe to use.
  Render::Mesh *GetMeshInstance() { return meshInstance; }

  template <typename Archive> void save(Archive &ar) const {
    ar(CEREAL_NVP(entityID));
    ar(meshInstance->modelPath, meshInstance->identifier);
  }

  template <typename Archive> void load(Archive &ar) {
    ar(CEREAL_NVP(entityID));
    std::string modelPath, identifier;
    ar(modelPath, identifier);
    meshInstance = Loader.GetMesh(modelPath, identifier);
    SetMeshInstance(meshInstance);
  }

  std::string getInspectorWindowName() override { return "Mesh"; }

  // VAO for mesh rendering
  Render::VAO vao;
  // VBO for rendering
  Render::Buffer target;
  // Mark whether the mesh is deformed
  bool Deformed = false;

private:
  friend class cereal::access;
  Render::Mesh *meshInstance = nullptr;
};

}; // namespace aEngine