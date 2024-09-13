#pragma once

#include <cmath>
#include <string>
#include <array>
#include <vector>
#include <stack>
#include <queue>
#include <iostream>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace aEngine {

namespace Spatial {

const float eps = 1e-8f;

struct Plane;
struct Line;
struct Capsule;
struct Sphere;
struct Ray;
struct Triangle;

struct AABB {
  glm::vec3 Min = glm::vec3(0.0f);
  glm::vec3 Max = glm::vec3(0.0f);

  bool Test(AABB &box);

  bool Test(Triangle &tri);

  glm::vec3 ClosestPointTo(glm::vec3 point);
  float PointDist(glm::vec3 point);
};

struct Plane {
  // Position, Normal
  glm::vec3 P, N;

  glm::vec3 ClosestPointTo(glm::vec3 point);
  float PointDist(glm::vec3 point);
};

struct Line {
  glm::vec3 Start, End;

  glm::vec3 ClosestPointTo(glm::vec3 point);
  float PointDist(glm::vec3 point);
};

struct Sphere {
  float R;
  glm::vec3 C;
};

struct Capsule {
  float R;
  glm::vec3 Start, End;
};

struct Ray {
  glm::vec3 Origin, Dir;

  bool Test(Triangle &tri, glm::vec3 &point);
};

struct Triangle {
  std::array<glm::vec3, 3> V;

  glm::vec3 Barycenter();

  bool Test(Triangle &tri);

  bool Test(Ray &ray, glm::vec3 &point);
};

}; // namespace Spatial

}; // namespace aEngine