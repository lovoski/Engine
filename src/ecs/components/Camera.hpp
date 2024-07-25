#pragma once

#include "global.hpp"
#include "ecs/ecs.hpp"
#include "ecs/components/Transform.hpp"

class Camera : public ECS::BaseComponent {
public:
  Camera() {}
  ~Camera() {}

  float zNear = 0.1f, zFar = 100.0f;
  float aspectRatio, fovY = 45.0f;
};