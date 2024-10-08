#pragma once

#include "Global.hpp"

namespace aEngine {

constexpr int MAX_BONES = 4;
struct Vertex {
  // Info related to rendering
  glm::vec4 Position;
  glm::vec4 Normal;
  // xy: uv1
  // zw: uv2 (if any)
  glm::vec4 TexCoords;
  // vertex color
  glm::vec4 Color;
  // Info related to skeleton animation
  int BoneId[MAX_BONES];
  float BoneWeight[MAX_BONES];
};

struct Texture {
  unsigned int id;
  std::string path;
};

namespace Render {
class Mesh;
class Shader;
class BasePass;
};

namespace Geometry {
class HalfedgeMesh;
};

namespace Animation {
class Motion;
};

}; // namespace aEngine