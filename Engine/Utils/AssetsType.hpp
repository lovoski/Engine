#pragma once

#include "Global.hpp"

namespace aEngine {

constexpr int MAX_BONE_INFLUENCER = 4;

struct Vertex {
  glm::vec3 Position;
  glm::vec3 Normal;
  glm::vec2 TexCoords;

  int BoneIds[MAX_BONE_INFLUENCER];
  float BoneWeights[MAX_BONE_INFLUENCER];
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