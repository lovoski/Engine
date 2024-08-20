#pragma once

#include "Global.hpp"
#include "Function/AssetsLoader.hpp"
#include "Function/AssetsType.hpp"
#include "Function/General/ComputeShader.hpp"
#include "Function/Render/Buffers.hpp"
#include "Function/Render/Shader.hpp"


namespace aEngine {

namespace Render {

class Mesh {
public:
  // mesh Data
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  VAO vao;
  Buffer vbo, ebo;

  Buffer defaultStates, skeletonTransforms;

  std::string identifier;
  std::string modelPath;

  // constructor
  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices) {
    this->vertices = vertices;
    this->indices = indices;
    setupMesh();
  }

  ~Mesh() {
    vao.Delete();
    vbo.Delete();
    ebo.Delete();
    defaultStates.Delete();
    skeletonTransforms.Delete();
  }

  // render the mesh
  void Draw(Shader &shader);

  void InitializeDeformData() {
    defaultStates.SetDataAs(GL_SHADER_STORAGE_BUFFER, vertices);
    defaultStates.UnbindAs(GL_SHADER_STORAGE_BUFFER);
  }

private:
  // initializes all the buffer objects/arrays
  void setupMesh();
};

}; // namespace Render

}; // namespace aEngine
