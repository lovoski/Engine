#include "Function/Animation/Deform.hpp"
#include "Function/AssetsLoader.hpp"
#include "Function/Render/Buffers.hpp"

namespace aEngine {

void BuildSkeletonHierarchy(Entity *root,
                            std::map<std::string, Entity *> &hierarchyMap) {
  std::queue<EntityID> q;
  q.push(root->ID);
  while (!q.empty()) {
    EntityID cur = q.front();
    Entity *curEnt = GWORLD.EntityFromID(cur);
    hierarchyMap.insert(std::make_pair(curEnt->name, curEnt));
    q.pop();
    for (auto c : curEnt->children)
      q.push(c->ID);
  }
}

static std::string skinnedMeshDeform = R"(
#version 430 core
layout(local_size_x = 8, local_size_y = 8, local_size_z = 1) in;
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

void DeformSkinnedMesh(Render::Mesh *mesh, Animator *animator,
                       Render::Buffer &targetVBO, Render::Buffer &matrices) {
  static ComputeShader cs(skinnedMeshDeform);
  cs.Use();
  // configure the inputs
  mesh->vbo.BindAs(GL_SHADER_STORAGE_BUFFER);
  mesh->vbo.BindToPointAs(GL_SHADER_STORAGE_BUFFER, 0);
  matrices.BindAs(GL_SHADER_STORAGE_BUFFER);
  matrices.SetDataAs(GL_SHADER_STORAGE_BUFFER,
                     animator->GetSkeletonTransforms());
  matrices.BindToPointAs(GL_SHADER_STORAGE_BUFFER, 1);
  // configure the outputs
  targetVBO.BindAs(GL_SHADER_STORAGE_BUFFER);
  targetVBO.BindToPointAs(GL_SHADER_STORAGE_BUFFER, 2);
  int numVertices = mesh->vertices.size();
  int numWorkGroups = (numVertices - (numVertices % 64)) / 64 + 1;
  if (glIsBuffer(mesh->vbo.GetID()) && glIsBuffer(targetVBO.GetID()) &&
      glIsBuffer(matrices.GetID()))
    cs.Dispatch(numWorkGroups, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

}; // namespace aEngine