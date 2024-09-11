#pragma once

#include <cmath>
#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace aEngine {

namespace Spatial {

struct AABB {
  glm::vec3 Min = glm::vec3(0.0f);
  glm::vec3 Max = glm::vec3(0.0f);

  bool Test(AABB &box);
};

}; // namespace Spatial

}; // namespace aEngine