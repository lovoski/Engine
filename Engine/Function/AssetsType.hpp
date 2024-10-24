#pragma once

#include "Global.hpp"

namespace aEngine {

struct BlendShapeVertex {
  glm::vec3 BlendShapeOffset;
  glm::vec3 BlendShapeNormal;
};
struct BlendShape {
  float weight = 0.0f;
  std::string name;
  std::vector<BlendShapeVertex> data;
};
const int MAX_BONES = 4;
const int MAX_BLEND_SHAPES = 52;
struct Vertex {
  glm::vec4 Position;
  glm::vec4 Normal;
  // uv1, uv2
  glm::vec4 TexCoords;
  glm::vec4 Color;

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