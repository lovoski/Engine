#include "Function/Render/Mesh.hpp"

namespace aEngine {

namespace Render {

void Mesh::setupMesh() {
  // load data into vertex buffers
  vbo.SetDataAs(GL_ARRAY_BUFFER, vertices);
  ebo.SetDataAs(GL_ELEMENT_ARRAY_BUFFER, indices);
  vbo.UnbindAs(GL_ARRAY_BUFFER);
  ebo.UnbindAs(GL_ELEMENT_ARRAY_BUFFER);
}

}; // namespace Render

}; // namespace aEngine