#pragma once

#include "Utils/AssetsLoader.hpp"
#include "Utils/AssetsType.hpp"
#include "Utils/General/ComputeShader.hpp"
#include "Utils/Render/Buffers.hpp"
#include "Utils/Render/Shader.hpp"


namespace aEngine {

namespace Render {

struct AnimVertex {
  glm::vec4 Position;
  glm::vec4 Normal;
  int BoneId[BONES];
  float BoneWeight[BONES];
};

const std::string skelAnimComp = R"(

)";

// class Deformable {
// public:
//   // mesh Data
//   std::vector<Vertex> vertices;
//   std::vector<unsigned int> indices;
//   std::vector<AnimData> animData;
//   VAO vao;
//   // Use ssbo to store the data
//   Buffer animVertices, boneTransforms;

//   std::string identifier;
//   std::string modelPath;

//   ComputeShader *cs;

//   // constructor
//   Deformable(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
//              std::vector<AnimData> aData) {
//     this->vertices = vertices;
//     this->indices = indices;
//     this->animData = aData;
//     cs = Loader.GetLoadedComputeShader("::skelAnim");
//     setupMesh();
//     Reset();
//   }

//   ~Deformable() {
//     vao.Delete();
//     vbo.Delete();
//     ebo.Delete();
//   }

//   // Reset the positions, normals to defualt states
//   void Reset() {
//     // construct AnimVertex array with existing data
//     std::vector<AnimVertex> animVertices;
//     for (int i = 0; i < vertices.size(); ++i) {
//       AnimVertex avertex;
//       avertex.Position = vertices[i].Position;
//       avertex.Normal = vertices[i].Normal;
//       for (int j = 0; j < BONES; ++j) {
//         avertex.BoneId[j] = animData[i].BoneId[j];
//         avertex.BoneWeight[j] = animData[i].BoneWeight[j];
//       }
//       animVertices.push_back(avertex);
//     }
//     animV.SetData(animVertices);
//   }

//   void SetBoneTransforms(std::vector<glm::mat4> &boneTransform) {
//     boneTrans.SetData(boneTransform);
//   }

//   // render the mesh
//   void Draw(Shader &shader) {
//     // transform the positions and normal first
//     cs->Use();
//     animV.Bind(
//         0); // positions, normals, bone indices and weights for each vertex
//     boneTrans.Bind(1); // transform matrices for each bone
//     cs->Dispatch(8, 8, 1);
//     // copy the result from ssbo to vbo
//     glBindBuffer(GL_COPY_READ_BUFFER, animV.GetID());
//     glBindBuffer(GL_COPY_WRITE_BUFFER, vbo.GetID());
//     // glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, );
//     glBindBuffer(GL_COPY_READ_BUFFER, 0);
//     glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
//     // draw mesh
//     shader.Use();
//     vao.Bind();
//     glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()),
//                    GL_UNSIGNED_INT, 0);
//     vao.Unbind();
//   }

// private:
//   Buffer vbo, ebo;

//   // initializes all the buffer objects/arrays
//   void setupMesh() {
//     vao.Bind();
//     // load data into vertex buffers
//     vbo.BindAs(GL_ARRAY_BUFFER);
//     ebo.BindAs(GL_ELEMENT_ARRAY_BUFFER);
//     vbo.SetDataAs(GL_ARRAY_BUFFER, vertices);
//     ebo.SetDataAs(GL_ELEMENT_ARRAY_BUFFER, indices);
//     vao.Unbind();
//     vbo.UnbindAs(GL_ARRAY_BUFFER);
//     ebo.UnbindAs(GL_ELEMENT_ARRAY_BUFFER);
//   }
// };

}; // namespace Render

}; // namespace aEngine
