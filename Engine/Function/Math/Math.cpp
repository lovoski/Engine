#include "Function/Math/Math.hpp"
#include <cmath>

using glm::cross;
using glm::dot;
using glm::normalize;
using glm::quat;
using glm::vec3;
using std::sqrt;

namespace aEngine {

namespace Math {

quat FromToRotation(vec3 from, vec3 to) {
  float from_norm = glm::length(from);
  float to_norm = glm::length(to);
  if (from_norm == 0.0f || to_norm == 0.0f)
    return quat(1.0f, vec3(0.0f));
  from = normalize(from);
  to = normalize(to);
  auto cosTheta = dot(from, to);
  if (abs(abs(cosTheta) - 1.0f) < 1e-9f) {
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

glm::vec3 FaceNormal(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2) {
  auto e01 = v1 - v0;
  auto e02 = v2 - v0;
  return glm::normalize(glm::cross(e01, e02));
}

void DecomposeTransform(const glm::mat4 &transform, glm::vec3 &outPosition,
                        glm::quat &outRotation, glm::vec3 &outScale) {
  glm::mat4 localMatrix(transform);

  // Extract the translation
  outPosition = glm::vec3(localMatrix[3]);

  // Extract the scale
  glm::vec3 scale;
  scale.x = glm::length(glm::vec3(localMatrix[0]));
  scale.y = glm::length(glm::vec3(localMatrix[1]));
  scale.z = glm::length(glm::vec3(localMatrix[2]));

  // Normalize the matrix columns to remove the scale from the rotation matrix
  if (scale.x != 0)
    localMatrix[0] /= scale.x;
  if (scale.y != 0)
    localMatrix[1] /= scale.y;
  if (scale.z != 0)
    localMatrix[2] /= scale.z;

  // Extract the rotation
  outRotation = glm::quat_cast(localMatrix);

  outScale = scale;
}

}; // namespace Math

}; // namespace aEngine