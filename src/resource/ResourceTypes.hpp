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

struct Texture {
  unsigned int id;
  string type;
  string path;
  string name;
};

enum PRIMITIVE_TYPE { CUBE, SPHERE, CYLINDER, PLANE };

enum ICON_TYPE { NULL_TYPE };

}; // namespace Resource