#pragma once

#include "Global.hpp"
#include "Utils/Animation/Motion.hpp"

namespace aEngine {

namespace VisUtils {

void DrawLine3D(glm::vec3 p0, glm::vec3 p1, glm::mat4 vp = glm::mat4(1.0f),
                glm::vec3 color = glm::vec3(1.0f), float thickness = 1.0f);

void DrawGrid(unsigned int gridSize, glm::mat4 mvp = glm::mat4(1.0f),
              glm::vec3 color = glm::vec3(1.0f));

void DrawSquare(glm::vec3 position, float size, glm::mat4 mvp = glm::mat4(1.0f),
                glm::vec2 viewport = glm::vec2(0.0f),
                glm::vec3 color = glm::vec3(1.0f));

void DrawBone(glm::vec3 start, glm::vec3 end,
              glm::vec2 viewport = glm::vec2(0.0f),
              glm::mat4 vp = glm::mat4(1.0f),
              glm::vec3 color = glm::vec3(0.0f, 1.0f, 0.0f));

}; // namespace VisUtils

}; // namespace aEngine