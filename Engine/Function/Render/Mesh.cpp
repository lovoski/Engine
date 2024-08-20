#include "Function/Render/Mesh.hpp"

namespace aEngine {

namespace Render {

void Mesh::setupMesh() {
  vao.Bind();
  // load data into vertex buffers
  vbo.SetDataAs(GL_ARRAY_BUFFER, vertices);
  ebo.SetDataAs(GL_ELEMENT_ARRAY_BUFFER, indices);
  // set the vertex attribute pointers
  vao.LinkAttrib(vbo, 0, 4, GL_FLOAT, sizeof(Vertex), (void *)0);
  vao.LinkAttrib(vbo, 1, 4, GL_FLOAT, sizeof(Vertex),
                 (void *)(offsetof(Vertex, Normal)));
  vao.LinkAttrib(vbo, 2, 4, GL_FLOAT, sizeof(Vertex),
                 (void *)(offsetof(Vertex, TexCoords)));

  vao.Unbind();
  vbo.UnbindAs(GL_ARRAY_BUFFER);
  ebo.UnbindAs(GL_ELEMENT_ARRAY_BUFFER);
}

void Mesh::Draw(Shader &shader) {
  // draw mesh
  shader.Use();
  vao.Bind();
  glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices.size()),
                 GL_UNSIGNED_INT, 0);
  vao.Unbind();
}

}; // namespace Render

}; // namespace aEngine