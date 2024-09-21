#include "Function/Math/Dampers.hpp"

namespace aEngine {

namespace Math {

#define M_PI 3.1415926535f

using glm::vec3;
using std::exp;

inline float FastNegExp(float x) {
  return 1.0f / (1.0f + x + 0.48f * x * x + 0.235f * x * x * x);
}

inline float FastAtan(float x) {
  float z = std::fabs(x);
  float w = z > 1.0f ? 1.0f / z : z;
  float y = (M_PI / 4.0f) * w - w * (w - 1) * (0.2447f + 0.0663f * w);
  return std::copysign(z > 1.0f ? M_PI / 2.0 - y : y, x);
}

inline float Square(float x) { return x * x; }

inline vec3 Lerp(vec3 &a, vec3 &b, float alpha) {
  return (1.0f - alpha) * a + alpha * b;
}

void DamperExp(float &position, float target, float dt, float halfLife) {
  float alpha = 1.0f - FastNegExp((0.69314718056f * dt) / (halfLife + 1e-8f));
  position = (1.0f - alpha) * position + alpha * target;
}
void DamperExp(vec3 &position, vec3 target, float dt, float halfLife) {
  position =
      Lerp(position, target,
           1.0f - FastNegExp((0.69314718056f * dt) / (halfLife + 1e-8f)));
}

void DamperSpring(float &x, float &v, float x_goal, float v_goal, float dt,
                  float stiffness, float damping) {
  static float eps = 1e-5f;
  float g = x_goal;
  float q = v_goal;
  float s = stiffness;
  float d = damping;
  float c = g + (d * q) / (s + eps);
  float y = d / 2.0f;

  if (fabs(s - (d * d) / 4.0f) < eps) {
    // Critically Damped
    float j0 = x - c;
    float j1 = v + j0 * y;

    float eydt = FastNegExp(y * dt);

    x = j0 * eydt + dt * j1 * eydt + c;
    v = -y * j0 * eydt - y * dt * j1 * eydt + j1 * eydt;
  } else if (s - (d * d) / 4.0f > 0.0) {
    // Under Damped
    float w = sqrtf(s - (d * d) / 4.0f);
    float j = sqrtf(Square(v + y * (x - c)) / (w * w + eps) + Square(x - c));
    float p = FastAtan((v + (x - c) * y) / (-(x - c) * w + eps));

    j = (x - c) > 0.0f ? j : -j;

    float eydt = FastNegExp(y * dt);

    x = j * eydt * cosf(w * dt + p) + c;
    v = -y * j * eydt * cosf(w * dt + p) - w * j * eydt * sinf(w * dt + p);
  } else if (s - (d * d) / 4.0f < 0.0) {
    // Over Damped
    float y0 = (d + sqrtf(d * d - 4 * s)) / 2.0f;
    float y1 = (d - sqrtf(d * d - 4 * s)) / 2.0f;
    float j1 = (c * y0 - x * y0 - v) / (y1 - y0);
    float j0 = x - j1 - c;

    float ey0dt = FastNegExp(y0 * dt);
    float ey1dt = FastNegExp(y1 * dt);

    x = j0 * ey0dt + j1 * ey1dt + c;
    v = -y0 * j0 * ey0dt - y1 * j1 * ey1dt;
  }
}

void DamperSpring(glm::vec3 &x, glm::vec3 &v, glm::vec3 pGoal, glm::vec3 vGoal,
                  float dt, float stiffness, float damping) {
  DamperSpring(x.x, v.x, pGoal.x, vGoal.x, dt, stiffness, damping);
  DamperSpring(x.y, v.y, pGoal.y, vGoal.y, dt, stiffness, damping);
  DamperSpring(x.z, v.z, pGoal.z, vGoal.z, dt, stiffness, damping);
}

}; // namespace Math

}; // namespace aEngine