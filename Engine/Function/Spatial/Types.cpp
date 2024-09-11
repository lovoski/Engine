#include "Function/Spatial/Types.hpp"

namespace aEngine {

namespace Spatial {

using glm::mat3;
using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;

bool operator<(vec3 a, vec3 b) { return a.x < b.x && a.y < b.y && a.z < b.z; }
bool operator>(vec3 a, vec3 b) { return a.x > b.x && a.y > b.y && a.z > b.z; }

bool AABB::Test(AABB &box) {
  if (Max.x < box.Min.x || Min.x > box.Max.x)
    return false;
  if (Max.y < box.Min.y || Min.y > box.Max.y)
    return false;
  if (Max.z < box.Min.z || Min.z > box.Max.z)
    return false;
  return true;
}

}; // namespace Spatial

}; // namespace aEngine