#include "Function/Animation/Deform.hpp"
#include "Function/AssetsLoader.hpp"
#include "Function/Render/Buffers.hpp"

namespace aEngine {

static std::string skinnedMeshDeform = R"(
#version 430 core
layout(local_size_x = 64, local_size_y = 1, local_size_z = 1) in;
#define MAX_BONES 4
struct Vertex {
  vec4 Position;
  vec4 Normal;
  vec4 TexCoords;
  vec4 Color;
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
  uint index = gl_GlobalInvocationID.x;
  if (index >= vIn.length()) return;
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
  // keep the rest property unchanged
  vOut[index] = vIn[index];
  // Store the transformed position and normal back in the SSBO
  vOut[index].Position = newPosition;
  vOut[index].Normal = normalize(newNormal);
}
)";

void DeformSkinnedMesh(Animator *animator, Render::Buffer &inputVBO,
                       unsigned int elementNum, Render::Buffer &targetVBO,
                       Render::Buffer &matrices) {
  static ComputeShader cs(skinnedMeshDeform);
  cs.Use();
  // configure the inputs
  inputVBO.BindAs(GL_SHADER_STORAGE_BUFFER);
  inputVBO.BindToPointAs(GL_SHADER_STORAGE_BUFFER, 0);
  matrices.BindAs(GL_SHADER_STORAGE_BUFFER);
  matrices.SetDataAs(GL_SHADER_STORAGE_BUFFER,
                     animator->GetSkeletonTransforms());
  auto mats = animator->GetSkeletonTransforms();
  matrices.BindToPointAs(GL_SHADER_STORAGE_BUFFER, 1);
  // configure the outputs
  targetVBO.BindAs(GL_SHADER_STORAGE_BUFFER);
  targetVBO.BindToPointAs(GL_SHADER_STORAGE_BUFFER, 2);
  int numVertices = elementNum;
  int numWorkGroups = (numVertices + 63) / 64;
  if (glIsBuffer(inputVBO.GetID()) && glIsBuffer(targetVBO.GetID()) &&
      glIsBuffer(matrices.GetID()))
    cs.Dispatch(numWorkGroups, 1, 1);
  glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
}

}; // namespace aEngine