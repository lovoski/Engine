#pragma once

#include "Function/AssetsLoader.hpp"
#include "Function/AssetsType.hpp"
#include "Function/General/ComputeShader.hpp"
#include "Function/Render/Buffers.hpp"
#include "Function/Render/Shader.hpp"
#include "Global.hpp"

namespace aEngine {

namespace Render {

class Mesh {
public:
  // mesh Data
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  VAO vao;
  Buffer vbo, ebo;

  Buffer defaultStates;

  std::string identifier;
  std::string modelPath;

  // Mark if the animation system is controlling the
  // transform of this mesh, if so, additional transform won't be applied
  // during rendering.
  bool AnimationPossesed = false;

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
