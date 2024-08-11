#include "Utils/Render/VisUtils.hpp"
#include "Utils/Math/Math.hpp"
#include "Utils/Render/Shader.hpp"


namespace aEngine {

namespace VisUtils {

void DrawLine3D(glm::vec3 p0, glm::vec3 p1, float thickness, glm::vec3 color,
                glm::mat4 vp) {
  static unsigned int vao, vbo;
  static bool openglObjectCreated = false;
  static Render::Shader *lineShader = new Render::Shader();
  static bool lineShaderLoaded = false;
  if (!lineShaderLoaded) {
    lineShader->LoadAndRecompileShader("./Assets/shaders/visutils/line.vert",
                                       "./Assets/shaders/visutils/line.frag");
    lineShaderLoaded = true;
  }
  if (!openglObjectCreated) {
    float point[] = {1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f};
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point), point, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),
                          (void *)0);
    glBindVertexArray(0);
    openglObjectCreated = true;
  }
  lineShader->Use();
  lineShader->SetVec3("color", color);
  glm::vec3 trans = (p0 + p1) * 0.5f;
  float scale = glm::length(p0 - p1) * 0.5f;
  glm::quat rot = Math::FromToRotation(glm::vec3(1.0f, 0.0f, 0.0f), p0 - p1);
  lineShader->SetMat4("mvp", vp * glm::translate(glm::mat4(1.0f), trans) *
                                 glm::mat4_cast(rot) *
                                 glm::scale(glm::mat4(1.0f), glm::vec3(scale)));
  glBindVertexArray(vao);
  glLineWidth(thickness);
  glDrawArrays(GL_LINES, 0, 2);
  glBindVertexArray(0);
}

template <typename T> struct PlainVertex {
  T x, y, z;
  PlainVertex(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
};

using PlainVertexi = PlainVertex<int>;
using PlainVertexf = PlainVertex<float>;

void DrawGrid(unsigned int gridSize, glm::vec3 color, glm::mat4 mvp) {
  static unsigned int vao, vbo;
  static int savedGridSize = -1;
  static bool openglObjectCreated = false;
  static std::vector<PlainVertexf> points;
  static Render::Shader *lineShader = new Render::Shader();
  static bool lineShaderLoaded = false;
  if (!lineShaderLoaded) {
    lineShader = new Render::Shader();
    lineShader->LoadAndRecompileShader("./Assets/shaders/visutils/line.vert",
                                       "./Assets/shaders/visutils/line.frag");
    lineShaderLoaded = true;
  }
  if (!openglObjectCreated) {
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);
    openglObjectCreated = true;
  }
  if (savedGridSize != gridSize) {
    // reallocate the data if grid size changes
    int current = gridSize;
    float sizef = static_cast<float>(gridSize);
    points.clear();
    while (current > 0) {
      float currentf = static_cast<float>(current);
      points.push_back({sizef, 0, currentf});
      points.push_back({-sizef, 0, currentf});
      points.push_back({sizef, 0, -currentf});
      points.push_back({-sizef, 0, -currentf});
      points.push_back({currentf, 0, sizef});
      points.push_back({currentf, 0, -sizef});
      points.push_back({-currentf, 0, sizef});
      points.push_back({-currentf, 0, -sizef});
      current--;
    }
    points.push_back({sizef, 0, 0});
    points.push_back({-sizef, 0, 0});
    points.push_back({0, 0, sizef});
    points.push_back({0, 0, -sizef});
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // allocate the buffer if grid size changed
    glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(PlainVertexf),
                 &points[0], GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glBindVertexArray(0);
    savedGridSize = gridSize;
  }
  lineShader->Use();
  lineShader->SetVec3("color", color);
  lineShader->SetMat4("mvp", mvp);
  glBindVertexArray(vao);
  // draw the normal grid lines
  glLineWidth(1.0f);
  glDrawArrays(GL_LINES, 0, points.size() - 4);
  // draw the axis lines
  glLineWidth(1.5f);
  glDrawArrays(GL_LINES, points.size() - 4, 4);
  glBindVertexArray(0);
}

}; // namespace VisUtils

}; // namespace aEngine