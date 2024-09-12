#include "Function/Spatial/Types.hpp"
#include "Function/Spatial/Library.hpp"

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

bool AABB::Test(Triangle &tri) {
  auto c = (Min + Max) * 0.5f;
  auto h = (Max - Min) * 0.5f;
  float center[3] = {c.x, c.y, c.z};
  float halfSize[3] = {h.x, h.y, h.z};
  float v[3][3] = {{tri.V[0].x, tri.V[0].y, tri.V[0].z},
                   {tri.V[1].x, tri.V[1].y, tri.V[1].z},
                   {tri.V[2].x, tri.V[2].y, tri.V[2].z}};
  return (bool)triBoxOverlap(center, halfSize, v);
}

glm::vec3 AABB::ClosestPointTo(glm::vec3 point) {
  glm::vec3 ret;
  for (int i = 0; i < 3; ++i) {
    float v = point[i];
    if (v < Min[i])
      v = Min[i];
    if (v > Max[i])
      v = Max[i];
    ret[i] = v;
  }
  return ret;
}

float AABB::PointDist(glm::vec3 point) {
  return glm::length(ClosestPointTo(point) - point);
}

glm::vec3 Plane::ClosestPointTo(glm::vec3 point) {
  N = glm::normalize(N);
  return point - glm::dot(point - P, N) * N;
}

float Plane::PointDist(glm::vec3 point) {
  N = glm::normalize(N);
  return glm::dot(point - P, N);
}

glm::vec3 Line::ClosestPointTo(glm::vec3 point) {
  float l = glm::length(End - Start);
  float p = glm::dot(point - Start, glm::normalize(End - Start));
  if (p < 0)
    return Start;
  else if (p > l)
    return End;
  else
    return Start + (End - Start) * (p / l);
}
float Line::PointDist(glm::vec3 point) {
  auto ab = End - Start, ap = point - Start, bp = point - End;
  float d = glm::dot(ap, ab);
  if (d <= 0.0f)
    return glm::length(ap);
  float f = glm::length(ab);
  if (d >= f)
    return glm::length(bp);
  return std::sqrt(glm::dot(ap, ap) - d * d);
}

bool Ray::Test(Triangle &tri, glm::vec3 &point) {
  double v[3][3] = {{tri.V[0].x, tri.V[0].y, tri.V[0].z},
                    {tri.V[1].x, tri.V[1].y, tri.V[1].z},
                    {tri.V[2].x, tri.V[2].y, tri.V[2].z}};
  double o[3] = {Origin.x, Origin.y, Origin.z};
  Dir = glm::normalize(Dir);
  double d[3] = {Dir.x, Dir.y, Dir.z};
  double x, y, z;
  if (intersect_triangle3(o, d, v[0], v[1], v[2], &x, &y, &z)) {
    point.x = x;
    point.y = y;
    point.z = z;
    return true;
  }
  return false;
}

bool Triangle::Test(Triangle &tri) {
  float v[3][3] = {{V[0].x, V[0].y, V[0].z},
                   {V[1].x, V[1].y, V[1].z},
                   {V[2].x, V[2].y, V[2].z}};
  float u[3][3] = {{tri.V[0].x, tri.V[0].y, tri.V[0].z},
                   {tri.V[1].x, tri.V[1].y, tri.V[1].z},
                   {tri.V[2].x, tri.V[2].y, tri.V[2].z}};
  return (bool)NoDivTriTriIsect(v[0], v[1], v[2], u[0], u[1], u[2]);
}

bool Triangle::Test(Ray &ray, glm::vec3 &point) {
  return ray.Test(*this, point);
}

glm::vec3 Triangle::Barycenter() { return (V[0] + V[1] + V[2]) / 3.0f; }

}; // namespace Spatial

}; // namespace aEngine