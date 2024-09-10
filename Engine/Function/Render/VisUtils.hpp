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

// `position` is a corner vertex
void DrawCube(glm::vec3 position, glm::vec3 forward, glm::vec3 left,
              glm::vec3 up, glm::mat4 vp = glm::mat4(1.0f),
              glm::vec3 size = glm::vec3(1.0f),
              glm::vec3 color = glm::vec3(0.0f, 1.0f, 0.0f));

void DrawWireSphere(glm::vec3 position, glm::mat4 vp, float radius = 1.0f,
                    glm::vec3 color = glm::vec3(0.0f, 1.0f, 0.0f));

void DrawArrow(glm::vec3 start, glm::vec3 end, glm::mat4 vp = glm::mat4(1.0f),
               glm::vec3 color = glm::vec3(1.0f), float size = 0.2f);

// Visualize a list of bones, with <start, end> pair.
void DrawBones(std::vector<std::pair<glm::vec3, glm::vec3>> &bones,
               glm::vec2 viewport, glm::mat4 vp = glm::mat4(1.0f),
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