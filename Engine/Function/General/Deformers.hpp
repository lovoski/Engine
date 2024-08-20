#pragma once

#include "Function/AssetsLoader.hpp"
#include "Function/General/ComputeShader.hpp"
#include "Function/Render/Buffers.hpp"
#include "Function/Render/Mesh.hpp"
#include "Component/Animator.hpp"

#include "Entity.hpp"

namespace aEngine {

// The deformers handles the deformation of a static mesh
// with compute shaders.
class BaseDeformer {
public:
  BaseDeformer(Render::Mesh *m) : mesh(m) {}
  ~BaseDeformer() {}

  virtual void DeformMesh(Entity *entity) {}

  virtual void DrawInspectorGUI() {
    drawInspectorGUIDefault();
  }

protected:
  Render::Mesh *mesh;

  void drawInspectorGUIDefault() {
    ImGui::MenuItem("Type:", nullptr, nullptr, false);
    ImGui::TextWrapped("%s", getMaterialTypeName().c_str());
    ImGui::MenuItem("Properties:", nullptr, nullptr, false);
  }

  virtual std::string getMaterialTypeName() { return typeid(*this).name(); }
};

const std::string skinnedMeshDeform = R"(
#version 430 core
)";

class SkinnedMeshDeformer : BaseDeformer {
public:
  SkinnedMeshDeformer(Render::Mesh *m) : BaseDeformer(m) {
    mesh = m;
    mesh->InitializeDeformData();
    cs = Loader.GetLoadedComputeShader("::skinnedMeshDeform");
  }
  ~SkinnedMeshDeformer() {}

  void DeformMesh(Entity *entity) override;

  void DrawInspectorGUI() override;

protected:
  ComputeShader *cs;
};

}; // namespace aEngine