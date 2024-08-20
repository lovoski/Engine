#include "Function/Math/Math.hpp"
#include <cmath>

using glm::quat;
using glm::vec3;
using glm::dot;
using glm::normalize;
using glm::cross;
using std::sqrt;

namespace aEngine {

namespace Math {

quat FromToRotation(vec3 from, vec3 to) {
  float from_norm = from.x * from.x + from.y * from.y + from.z * from.z;
  float to_norm = to.x * to.x + to.y * to.y + to.z * to.z;
  if (from_norm == 0.0f || to_norm == 0.0f)
    return quat(1.0f, vec3(0.0f));
  from = from / sqrt(from_norm);
  to = to / sqrt(to_norm);
  auto cosTheta = dot(from, to);
  if (abs(cosTheta - 1.0f) < 1e-4f) {
    return quat(1.0f, vec3(0.0f));
  }
  auto normal = normalize(cross(from, to));
  float theta = acos(cosTheta);
  return quat(cosf(theta / 2), sinf(theta / 2) * normal);
}

quat LookAtRotation(vec3 forward, vec3 up) {
  forward = normalize(forward);
  up = normalize(up);
  vec3 left = normalize(cross(up, forward));
  up = normalize(cross(forward, left));
  glm::mat3 rot(left, up, forward);
  return glm::quat_cast(rot);
}

};

};