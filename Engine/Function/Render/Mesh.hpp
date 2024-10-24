#pragma once

#include "Function/AssetsLoader.hpp"
#include "Function/AssetsType.hpp"
#include "Function/General/ComputeShader.hpp"
#include "Function/Render/Buffers.hpp"
#include "Function/Render/Shader.hpp"
#include "Global.hpp"

namespace aEngine::Render {

// Each mesh object holds a initial opengl buffer object
class Mesh {
public:
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;

  std::vector<BlendShape> blendShapes;
  Buffer vbo, ebo;

  std::string identifier;
  std::string modelPath;

  // constructor
  Mesh(std::vector<Vertex> &vertices, std::vector<unsigned int> &indices) {
    this->vertices = vertices;
    this->indices = indices;
    setupMesh();
  }
  ~Mesh() {}

private:
  // initializes all the buffer objects/arrays
  void setupMesh();
};

}; // namespace aEngine
