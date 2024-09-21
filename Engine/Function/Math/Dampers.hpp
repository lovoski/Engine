/**
 * Thanks to: https://theorangeduck.com/page/spring-roll-call
 */
#pragma once

#include "Global.hpp"

namespace aEngine {

namespace Math {

// Approach `position` to `target`, the `halfLife` parameter controls
// how fast the `position` approaches `target`.
// This method don't guarantee velocity continuity.
void DamperExact(glm::vec3 &position, glm::vec3 target, float dt,
                 float halfLife);

// Approach `position` to `target`, higher `damping` make the damper less
// responsive, while higher `stiffness` makes the spring more powerful.
void DamperSpring(glm::vec3 &position, glm::vec3 &velocity, glm::vec3 pGoal,
                  glm::vec3 vGoal, float dt, float stiffness = 5.0f,
                  float damping = 20.0f);

}; // namespace Math

}; // namespace aEngine