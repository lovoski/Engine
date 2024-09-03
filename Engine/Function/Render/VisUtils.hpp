#pragma once

#include "Function/Animation/Motion.hpp"
#include "Function/Render/Buffers.hpp"
#include "Global.hpp"

namespace aEngine {

namespace VisUtils {

// Draw GL_LINE_STRIP primitive
void DrawLineStrip3D(std::vector<glm::vec3> &lineStrip,
                     glm::mat4 vp = glm::mat4(1.0f),
                     glm::vec3 color = glm::vec3(1.0f), float thickness = 1.0f);

void DrawGrid(unsigned int gridSize, unsigned int gridSpacing,
              glm::mat4 mvp = glm::mat4(1.0f),
              glm::vec3 color = glm::vec3(1.0f));

void DrawSquare(glm::vec3 position, float size, glm::mat4 mvp = glm::mat4(1.0f),
                glm::vec2 viewport = glm::vec2(0.0f),
                glm::vec3 color = glm::vec3(0.5f));

// `position` is a corner vertex, fd, ld and ud are length along the directions
void DrawCube(glm::vec3 position, glm::vec3 forward, glm::vec3 left,
              glm::vec3 up, glm::mat4 vp = glm::mat4(1.0f), float fd = 1.0f,
              float ld = 1.0f, float ud = 1.0f,
              glm::vec3 color = glm::vec3(0.0f, 1.0f, 0.0f));

void DrawArrow(glm::vec3 start, glm::vec3 end, glm::mat4 vp = glm::mat4(1.0f),
               glm::vec3 color = glm::vec3(1.0f), float size = 0.2f);

void DrawBone(glm::vec3 start, glm::vec3 end,
              glm::vec2 viewport = glm::vec2(0.0f),
              glm::mat4 vp = glm::mat4(1.0f),
              glm::vec3 color = glm::vec3(0.0f, 1.0f, 0.0f));

// Visualize the directional light component, the direction for
// DIRECTIONAL_LIGHT is LocalForward
void DrawDirectionalLight(glm::vec3 forward, glm::vec3 up, glm::vec3 left,
                          glm::vec3 pos, glm::mat4 vp = glm::mat4(1.0f),
                          float size = 0.5f);

void DrawPointLight(glm::vec3 pos, glm::mat4 vp = glm::mat4(1.0f),
                    float size = 0.5f);

void DrawCamera(glm::vec3 forward, glm::vec3 up, glm::vec3 left, glm::vec3 pos,
                glm::mat4 vp, float fovY, float aspect, float size = 0.5f);

}; // namespace VisUtils

}; // namespace aEngine