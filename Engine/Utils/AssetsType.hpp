#pragma once

#include "Global.hpp"

namespace aEngine {

struct Vertex {
  glm::vec4 Position;
  glm::vec4 Normal;
  glm::vec2 TexCoords;
};

constexpr int BONES = 4;
struct AnimData {
  int BoneId[BONES];
  float BoneWeight[BONES];
};

struct Texture {
  unsigned int id;
  std::string path;
};

namespace Render {
class Mesh;
class Shader;
class BaseMaterial;
};

namespace Geometry {
class HalfedgeMesh;
};

namespace Animation {
class Motion;
};

}; // namespace aEngine