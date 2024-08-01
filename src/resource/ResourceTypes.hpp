#pragma once

#include "global.hpp"

namespace Resource {

struct Vertex {
  vec3 Position;
  vec3 Normal;
  vec2 TexCoords;
  // vec3 Tangent;
  // vec3 Bitangent;
};

enum TEXTURE_TYPE { ICON_TEXTURE, IMAGE_TEXTURE };

struct Texture {
  unsigned int id;
  string path;
  TEXTURE_TYPE type;
};

enum PRIMITIVE_TYPE { CUBE, SPHERE, CYLINDER, PLANE, CONE };

enum ICON_TYPE { NULL_TYPE };

}; // namespace Resource