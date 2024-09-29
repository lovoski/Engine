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
  Mesh() : BaseComponent(-1) {}
  Mesh(EntityID id, Render::Mesh *mesh);

  void DrawInspectorGUI() override;

  // Bind vao, ready for rendering
  void Bind();
  // Unbind vao, finish rendering
  void Unbind();

  // Setup bvh with member variable meshInstance, this function
  // will rebuild instead of updating the bvh.
  void BuildBVH(glm::mat4 &transform);

  void SetMeshInstance(Render::Mesh *mesh);
  // The mesh instance is not allowed to be nullptr,
  // this function should be safe to use.
  Render::Mesh *GetMeshInstance() { return meshInstance; }

  template <typename Archive>
  void serialize(Archive &ar, const unsigned int version) {
    ar &boost::serialization::base_object<BaseComponent>(*this);
  }

  std::string getInspectorWindowName() override { return "Mesh"; }

  // VAO for mesh rendering
  Render::VAO vao;
  // VBO for rendering
  Render::Buffer target;
  // Mark whether the mesh is deformed
  bool Deformed = false;

  // When set to false, the bvh will be cleared, the object 
  // won't be considered in collision as well.
  bool AsCollider = false;
  // When set to false, the bvh will gets updated in each 
  // PreUpdate of SpatialSystem.
  bool StaticCollider = true;

  // bounding volumn hierarchy
  Spatial::BVH bvh;

  // Visualize only the bvh leaf nodes
  bool DrawLeafNodeOnly = true;

private:
  Render::Mesh *meshInstance = nullptr;

  void buildBVHDeformed();
  void buildBVHOriginal(glm::mat4 transform);

};

}; // namespace aEngine