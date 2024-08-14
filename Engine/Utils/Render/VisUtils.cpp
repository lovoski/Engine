#include "Utils/Render/VisUtils.hpp"
#include "Utils/Math/Math.hpp"
#include "Utils/Render/Shader.hpp"

namespace aEngine {

namespace VisUtils {

std::string lineVS = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
uniform mat4 mvp;
void main() {
  gl_Position = mvp * vec4(aPos, 1.0);
}
)";
std::string lineFS = R"(
#version 460 core
uniform vec3 color;
out vec4 FragColor;
void main() {
  FragColor = vec4(color, 1.0);
}
)";

// TODO: the width property don't work on my device, use geometry shader
void DrawLine3D(glm::vec3 p0, glm::vec3 p1, glm::mat4 vp, glm::vec3 color,
                float thickness) {
  static unsigned int vao, vbo;
  static bool openglObjectCreated = false;
  static Render::Shader *lineShader = new Render::Shader();
  static bool lineShaderLoaded = false;
  if (!lineShaderLoaded) {
    lineShader->LoadAndRecompileShaderSource(lineVS, lineFS);
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

// TODO: some shadow-ish issue with the code
void DrawGrid(unsigned int gridSize, glm::mat4 mvp, glm::vec3 color) {
  static unsigned int vao, vbo;
  static int savedGridSize = -1;
  static bool openglObjectCreated = false;
  static std::vector<PlainVertexf> points;
  static Render::Shader *lineShader = new Render::Shader();
  static bool lineShaderLoaded = false;
  if (!lineShaderLoaded) {
    lineShader = new Render::Shader();
    lineShader->LoadAndRecompileShaderSource(lineVS, lineFS);
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
  glDrawArrays(GL_LINES, 0, points.size() - 4);
  // draw the axis lines
  glDrawArrays(GL_LINES, points.size() - 4, 4);
  glBindVertexArray(0);
}

std::string squareVS = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
uniform mat4 mvp;
void main() {
  gl_Position = mvp * vec4(aPos, 1.0);
}
)";
std::string squareGS = R"(
#version 460 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;
uniform vec2 viewportSize;
uniform float size;
void main() {
  float r = viewportSize.y/viewportSize.x;
  float s = size * 0.1;
  gl_Position = gl_in[0].gl_Position+vec4(r*s, s, 0.0, 0.0);
  EmitVertex();
  gl_Position = gl_in[0].gl_Position+vec4(r*s, -s, 0.0, 0.0);
  EmitVertex();
  gl_Position = gl_in[0].gl_Position+vec4(-r*s, s, 0.0, 0.0);
  EmitVertex();
  gl_Position = gl_in[0].gl_Position+vec4(-r*s, -s, 0.0, 0.0);
  EmitVertex();
  EndPrimitive();
}
)";
std::string squareFS = R"(
#version 460 core
uniform vec3 color;
out vec4 FragColor;
void main() {
  FragColor = vec4(color, 1.0);
}
)";
void DrawSquare(glm::vec3 position, float size, glm::mat4 mvp,
                glm::vec2 viewportSize, glm::vec3 color) {
  static unsigned int vao, vbo;
  static Render::Shader *squareShader = new Render::Shader();
  static bool shaderLoaded = false;
  static glm::vec3 savedPos(-std::numeric_limits<float>::max(), 0, 0);
  if (!shaderLoaded) {
    squareShader->LoadAndRecompileShaderSource(squareVS, squareFS, squareGS);
    shaderLoaded = true;
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);
  }
  if (savedPos != position) {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    float positions[] = {position.x, position.y, position.z};
    glBufferData(GL_ARRAY_BUFFER, sizeof(positions), positions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glBindVertexArray(0);
  }
  squareShader->Use();
  squareShader->SetVec3("color", color);
  squareShader->SetVec2("viewportSize", viewportSize);
  squareShader->SetMat4("mvp", mvp);
  squareShader->SetFloat("size", size);
  glBindVertexArray(vao);
  glDrawArrays(GL_POINTS, 0, 1);
  glBindVertexArray(0);
}

std::string boneVS = R"(
#version 460 core
layout (location = 0) in vec3 aPos;
uniform mat4 mvp;
void main() {
  gl_Position = mvp * vec4(aPos, 1.0);
}
)";
std::string boneGS = R"(
#version 460 core
layout (lines) in;
layout (triangle_strip, max_vertices = 14) out;

uniform vec2 viewportSize;
out vec4 fragPos; // Output to the fragment shader

void main() {
  float aspectRatio = viewportSize.y / viewportSize.x;
  float size = length(gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz) * 0.05;
  vec3 boneDir = normalize(gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz);

  vec3 ortho1 = normalize(cross(boneDir, vec3(0.0, 0.0, 1.0)));
  vec3 ortho2 = cross(boneDir, ortho1);

  vec4 p0 = gl_in[0].gl_Position;
  vec4 p1 = gl_in[1].gl_Position;

  for (int i = 0; i < 6; ++i) {
    float angle = 2.0 * 3.14159265 * i / 5.0;
    vec3 offset = ortho1 * cos(angle) * size + ortho2 * sin(angle) * size;

    gl_Position = p0 + vec4(offset, 0.0);
    fragPos = gl_Position; // Pass the position to the fragment shader
    EmitVertex();

    gl_Position = p1 + vec4(offset * 0.5, 0.0);
    fragPos = gl_Position; // Pass the position to the fragment shader
    EmitVertex();
  }
  EndPrimitive();

  gl_Position = p0;
  fragPos = gl_Position; // Pass the position to the fragment shader
  EmitVertex();

  gl_Position = p1;
  fragPos = gl_Position; // Pass the position to the fragment shader
  EmitVertex();

  EndPrimitive();
}
)";
std::string boneFS = R"(
#version 460 core
uniform vec3 color;
in vec4 fragPos; // Input from the geometry shader
out vec4 FragColor;

void main() {
  // Calculate depth-based shading
  float depth = gl_FragCoord.z / gl_FragCoord.w;
  float factor = clamp(1.0 - depth, 0.0, 1.0);

  // Mix the color based on the depth
  vec3 shadedColor = mix(color * 0.5, color, factor);

  FragColor = vec4(shadedColor, 1.0);
}
)";
void DrawBone(glm::vec3 start, glm::vec3 end, glm::vec2 viewport, glm::mat4 vp,
              glm::vec3 color) {
  static unsigned int vao, vbo;
  static bool openglObjectCreated = false;
  static Render::Shader *boneShader = new Render::Shader();
  static bool lineShaderLoaded = false;
  if (!lineShaderLoaded) {
    boneShader->LoadAndRecompileShaderSource(boneVS, boneFS, boneGS);
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
  boneShader->Use();
  boneShader->SetVec3("color", color);
  boneShader->SetVec2("viewportSize", viewport);
  glm::vec3 trans = (start + end) * 0.5f;
  float scale = glm::length(start - end) * 0.5f;
  glm::quat rot =
      Math::FromToRotation(glm::vec3(1.0f, 0.0f, 0.0f), start - end);
  boneShader->SetMat4("mvp", vp * glm::translate(glm::mat4(1.0f), trans) *
                                glm::mat4_cast(rot) *
                                glm::scale(glm::mat4(1.0f), glm::vec3(scale)));
  glBindVertexArray(vao);
  glDrawArrays(GL_LINES, 0, 2);
  glBindVertexArray(0);
}

}; // namespace VisUtils

}; // namespace aEngine