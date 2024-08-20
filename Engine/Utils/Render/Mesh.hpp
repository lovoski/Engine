#pragma once

#include "Global.hpp"
#include "Utils/AssetsType.hpp"
#include "Utils/Render/Shader.hpp"
#include "Utils/Render/Buffers.hpp"

namespace aEngine {

namespace Render {

class Mesh {
public:
  // mesh Data
  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  VAO vao;

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
  }

  // render the mesh
  void Draw(Shader &shader) {
    // draw mesh
    shader.Use();
    vao.Bind();
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()),
                   GL_UNSIGNED_INT, 0);
    vao.Unbind();
  }

private:
  Buffer vbo, ebo;

  // initializes all the buffer objects/arrays
  void setupMesh() {
    vao.Bind();
    // load data into vertex buffers
    vbo.SetDataAs(GL_ARRAY_BUFFER, vertices);
    ebo.SetDataAs(GL_ELEMENT_ARRAY_BUFFER, indices);
    // set the vertex attribute pointers
    vao.LinkAttrib(vbo, 0, 4, GL_FLOAT, sizeof(Vertex), (void*)0);
    vao.LinkAttrib(vbo, 1, 4, GL_FLOAT, sizeof(Vertex), (void*)(offsetof(Vertex, Normal)));
    vao.LinkAttrib(vbo, 2, 2, GL_FLOAT, sizeof(Vertex), (void*)(offsetof(Vertex, TexCoords)));

    vao.Unbind();
    vbo.UnbindAs(GL_ARRAY_BUFFER);
    ebo.UnbindAs(GL_ELEMENT_ARRAY_BUFFER);
  }
};

}; // namespace Render

}; // namespace aEngine
