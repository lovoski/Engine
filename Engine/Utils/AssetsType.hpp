#pragma once

#include "Global.hpp"

namespace aEngine {

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;
};

struct Texture {
  unsigned int id;
  std::string path;
};

namespace Render {
class Mesh;
class Shader;
class MaterialData;
};

namespace Geometry {
class HalfedgeMesh;
};

namespace Animation {
class Motion;
};

}; // namespace aEngine