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

  // Deform the mesh with specified rules,
  // if freshStart is true, the input for compute shader should be 
  // defaultStates from the mesh, otherwise it should be vbo directly
  virtual void DeformMesh(Entity *entity, bool freshStart) {}

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
layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
#define MAX_BONES 4
struct Vertex {
  vec4 Position;
  vec4 Normal;
  vec4 TexCoords;
  int BoneId[MAX_BONES];
  float BoneWeight[MAX_BONES];
};
layout(std430, binding = 0) buffer VertexInput {
  Vertex vIn[];
};
layout(std430, binding = 1) buffer BoneTransforms {
  mat4 boneMatrices[];
};
layout(std430, binding = 2) buffer VertexOutput {
  Vertex vOut[];
};
void main() {
  uint index = gl_GlobalInvocationID.x + 
               gl_GlobalInvocationID.y * gl_NumWorkGroups.x * gl_WorkGroupSize.x + 
               gl_GlobalInvocationID.z * gl_NumWorkGroups.x * gl_WorkGroupSize.x * 
                                         gl_NumWorkGroups.y * gl_WorkGroupSize.y;

  Vertex vtx = vIn[index];
  // Initialize the new position and normal
  vec4 newPosition = vec4(0.0);
  vec4 newNormal = vec4(0.0);
  // Apply skinning
  for (int i = 0; i < MAX_BONES; i++) {
    int boneId = vtx.BoneId[i];
    float weight = vtx.BoneWeight[i];
    // Skip if the weight is zero
    if (weight > 0.0) {
      mat4 boneMatrix = boneMatrices[boneId];
      newPosition += boneMatrix * vtx.Position * weight;
      newNormal += boneMatrix * vtx.Normal * weight;
    }
  }
  // Store the transformed position and normal back in the SSBO
  vOut[index].Position = newPosition;
  vOut[index].Normal = normalize(newNormal);
}
)";

class SkinnedMeshDeformer : BaseDeformer {
public:
  SkinnedMeshDeformer(Render::Mesh *m) : BaseDeformer(m) {
    mesh = m;
    mesh->InitializeDeformData();
    cs = Loader.GetLoadedComputeShader("::skinnedMeshDeform");
  }
  ~SkinnedMeshDeformer() {}

  void DeformMesh(Entity *entity, bool freshStart = false) override;

  void DrawInspectorGUI() override;

protected:
  ComputeShader *cs;
};

}; // namespace aEngine