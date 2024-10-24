#include "Function/Render/VisUtils.hpp"
#include "Function/Math/Math.hpp"
#include "Function/Render/Shader.hpp"

namespace aEngine {

namespace VisUtils {

glm::vec3 Red = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 Green = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 Blue = glm::vec3(0.0f, 0.0f, 1.0f);
glm::vec3 Yellow = glm::vec3(1.0f, 1.0f, 0.0f);
glm::vec3 Purple = glm::vec3(1.0f, 0.0f, 1.0f);
glm::vec3 White = glm::vec3(1.0f);
glm::vec3 Grey = glm::vec3(0.5f);
glm::vec3 Black = glm::vec3(0.0f);

std::string lineVS = R"(
#version 430 core
layout (location = 0) in vec3 aPos;
uniform mat4 mvp;
void main() {
  gl_Position = mvp * vec4(aPos, 1.0);
}
)";
std::string lineFS = R"(
#version 430 core
uniform vec3 color;
out vec4 FragColor;
void main() {
  FragColor = vec4(color, 1.0);
}
)";

void DrawLineStrip3D(std::vector<glm::vec3> &lineStrip, glm::mat4 vp,
                     glm::vec3 color, float thickness) {
  static Render::VAO vao;
  static Render::Buffer vbo;
  static Render::Shader *lineShader = new Render::Shader();
  static bool shaderLoaded = false;
  if (!shaderLoaded) {
    lineShader->LoadAndRecompileShaderSource(lineVS, lineFS);
    shaderLoaded = true;
  }
  // update data in buffer every time
  vao.Bind();
  vbo.BindAs(GL_ARRAY_BUFFER);
  vbo.SetDataAs(GL_ARRAY_BUFFER, lineStrip, GL_STATIC_DRAW);
  vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(glm::vec3), (void *)0);
  lineShader->Use();
  lineShader->SetMat4("mvp", vp);
  lineShader->SetVec3("color", color);
  glDrawArrays(GL_LINE_STRIP, 0, lineStrip.size());
  vao.Unbind();
  vbo.UnbindAs(GL_ARRAY_BUFFER);
}

void DrawLines(std::vector<std::pair<glm::vec3, glm::vec3>> &lines,
               glm::mat4 vp, glm::vec3 color) {
  static Render::VAO vao;
  static Render::Buffer vbo;
  static Render::Shader lineShader;
  static bool shaderLoaded = false;
  if (!shaderLoaded) {
    lineShader.LoadAndRecompileShaderSource(lineVS, lineFS);
    shaderLoaded = true;
  }
  // update data in buffer every time
  vao.Bind();
  vbo.BindAs(GL_ARRAY_BUFFER);
  std::vector<glm::vec3> linePoints;
  for (auto &pointPair : lines) {
    linePoints.push_back(pointPair.first);
    linePoints.push_back(pointPair.second);
  }
  vbo.SetDataAs(GL_ARRAY_BUFFER, linePoints, GL_STATIC_DRAW);
  vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, sizeof(glm::vec3), (void *)0);
  lineShader.Use();
  lineShader.SetMat4("mvp", vp);
  lineShader.SetVec3("color", color);
  glDrawArrays(GL_LINES, 0, linePoints.size());
  vao.Unbind();
  vbo.UnbindAs(GL_ARRAY_BUFFER);
}

template <typename T> struct PlainVertex {
  T x, y, z;
  PlainVertex(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
};

using PlainVertexi = PlainVertex<int>;
using PlainVertexf = PlainVertex<float>;

// TODO: some shadow-ish issue with the code
void DrawGrid(unsigned int gridSize, unsigned int gridSpacing, glm::mat4 mvp,
              glm::vec3 color) {
  static Render::VAO vao;
  static Render::Buffer vbo;
  static int savedGridSize = -1, savedGridSpacing = -1;
  static std::vector<PlainVertexf> points;
  static Render::Shader lineShader;
  static bool lineShaderLoaded = false;
  if (!lineShaderLoaded) {
    lineShader.LoadAndRecompileShaderSource(lineVS, lineFS);
    lineShaderLoaded = true;
  }
  if (savedGridSize != gridSize || savedGridSpacing != gridSpacing) {
    // reallocate the data if grid size changes
    int current = gridSize;
    int spacing = gridSpacing;
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
      current -= spacing;
    }
    points.push_back({sizef, 0, 0});
    points.push_back({-sizef, 0, 0});
    points.push_back({0, 0, sizef});
    points.push_back({0, 0, -sizef});
    vao.Bind();
    vbo.SetDataAs(GL_ARRAY_BUFFER, points);
    vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, 3 * sizeof(float), (void *)0);
    vao.Unbind();
    vbo.UnbindAs(GL_ARRAY_BUFFER);
    savedGridSize = gridSize;
    savedGridSpacing = gridSpacing;
  }
  lineShader.Use();
  lineShader.SetVec3("color", color);
  lineShader.SetMat4("mvp", mvp);
  vao.Bind();
  // draw the normal grid lines
  glDrawArrays(GL_LINES, 0, points.size() - 4);
  // draw the axis lines
  glDrawArrays(GL_LINES, points.size() - 4, 4);
  vao.Unbind();
}

std::string squareVS = R"(
#version 430 core
layout (location = 0) in vec3 aPos;
uniform mat4 mvp;
void main() {
  gl_Position = mvp * vec4(aPos, 1.0);
}
)";
std::string squareGS = R"(
#version 430 core
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
void DrawSquares(std::vector<glm::vec3> positions, float size, glm::mat4 vp,
                 glm::vec2 viewport, glm::vec3 color) {
  static Render::VAO vao;
  static Render::Buffer vbo;
  static Render::Shader squareShader;
  static bool shaderLoaded = false;
  if (!shaderLoaded) {
    squareShader.LoadAndRecompileShaderSource(squareVS, squareFS, squareGS);
    shaderLoaded = true;
  }
  vao.Bind();
  vbo.SetDataAs(GL_ARRAY_BUFFER, positions);
  vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, 3 * sizeof(float), 0);
  squareShader.Use();
  squareShader.SetVec3("color", color);
  squareShader.SetVec2("viewportSize", viewport);
  squareShader.SetMat4("mvp", vp);
  squareShader.SetFloat("size", size);
  glDrawArrays(GL_POINTS, 0, positions.size());
  vao.Unbind();
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

std::string boneVS = R"(
#version 330 core
layout(location = 0) in vec3 aPos;

void main() {
    gl_Position = vec4(aPos, 1.0);
}
)";
std::string boneGS = R"(
#version 330 core
layout(lines) in;
layout(line_strip, max_vertices = 24) out;

uniform mat4 mvp;
uniform float radius;

void main() {
    vec3 start = gl_in[0].gl_Position.xyz;
    vec3 end = gl_in[1].gl_Position.xyz;

    // Compute direction and orthogonal vectors
    vec3 dir = end - start;
    float boneLength = length(dir);
    float normalOffset = radius * boneLength;
    vec3 direction = normalize(dir);
    vec3 up = normalize(vec3(0.0, 1.0, 1e-3));
    vec3 right = normalize(cross(direction, up));
    float dirOffset = 0.17;
    up = cross(right, direction);

    // Define the four offset vectors for the octahedral shape
    vec3 offsets[4];
    offsets[0] = right * normalOffset + dirOffset * dir;
    offsets[1] = up * normalOffset + dirOffset * dir;
    offsets[2] = -right * normalOffset + dirOffset * dir;
    offsets[3] = -up * normalOffset + dirOffset * dir;

    // Generate the edges of the octahedron
    for (int i = 0; i < 4; ++i) {
        // Lines connecting start point to the offset points
        gl_Position = mvp * vec4(start, 1.0);
        EmitVertex();
        gl_Position = mvp * vec4(start + offsets[i], 1.0);
        EmitVertex();
        EndPrimitive();

        gl_Position = mvp * vec4(start + offsets[(i+1)%4], 1.0);
        EmitVertex();
        gl_Position = mvp * vec4(start + offsets[i], 1.0);
        EmitVertex();
        EndPrimitive();

        // Lines connecting the start and end offset points
        gl_Position = mvp * vec4(start + offsets[i], 1.0);
        EmitVertex();
        gl_Position = mvp * vec4(end, 1.0);
        EmitVertex();
        EndPrimitive();
    }
}
)";
std::string boneFS = R"(
#version 330 core
out vec4 FragColor;

uniform vec3 color; // Color of the wireframe

void main() {
    FragColor = vec4(color, 1.0);
}
)";
void DrawBones(std::vector<std::pair<glm::vec3, glm::vec3>> &bones,
               glm::vec2 viewport, glm::mat4 vp, glm::vec3 color) {
  static Render::VAO vao;
  static Render::Buffer vbo;
  static Render::Shader boneShader;
  static bool shaderInitialized = false;
  if (!shaderInitialized) {
    boneShader.LoadAndRecompileShaderSource(boneVS, boneFS, boneGS);
    shaderInitialized = true;
  }
  // initialize vbo with `bones`
  vao.Bind();
  std::vector<glm::vec3> buffer;
  for (auto &pair : bones) {
    buffer.push_back(pair.first);  // bone start
    buffer.push_back(pair.second); // bond end
  }
  vbo.SetDataAs(GL_ARRAY_BUFFER, buffer);
  vao.LinkAttrib(vbo, 0, 3, GL_FLOAT, 3 * sizeof(float), (void *)0);
  boneShader.Use();

  boneShader.SetFloat("radius", 0.1f);
  boneShader.SetVec3("color", color);
  boneShader.SetVec2("viewportSize", viewport);
  boneShader.SetMat4("mvp", vp);

  glDrawArrays(GL_LINES, 0, buffer.size());

  vao.Unbind();
  vbo.UnbindAs(GL_ARRAY_BUFFER);
}

void DrawArrow(glm::vec3 start, glm::vec3 end, glm::mat4 vp, glm::vec3 color,
               float size) {
  static int segs = 12;
  glm::vec3 dir = glm::normalize(end - start);
  glm::vec3 normal =
      glm::normalize(glm::cross(dir, glm::vec3(0.0f, 1.0f, 0.0f)));
  std::vector<glm::vec3> strip1{start};
  std::vector<glm::vec3> strip2{end + normal * size};
  float rotDegree = glm::radians(360.0f / segs);
  auto lastWalkDir = normal;
  for (int i = 0; i < segs; ++i) {
    auto walkDir = glm::angleAxis(rotDegree * (i + 1), dir) * normal;
    strip1.push_back(end);
    strip1.push_back(end + lastWalkDir * size);
    strip1.push_back(end + dir * size * 1.8f);
    strip1.push_back(end + walkDir * size);
    strip2.push_back(end + walkDir * size);
    lastWalkDir = walkDir;
  }
  DrawLineStrip3D(strip1, vp, color);
  DrawLineStrip3D(strip2, vp, color);
}

void DrawDirectionalLight(glm::vec3 forward, glm::vec3 up, glm::vec3 left,
                          glm::vec3 pos, glm::mat4 vp, float size) {
  // construct the line strip needed
  std::vector<glm::vec3> strip1{
      pos + left * size * 1.5f - up * size,
      pos - left * size * 1.5f + up * size,
      pos - left * size * 1.5f - up * size,
      pos + left * size * 1.5f + up * size,
  };
  std::vector<glm::vec3> strip2{
      pos - left * size * 1.5f - up * size,
      pos + left * size * 1.5f - up * size,
      pos + left * size * 1.5f + up * size,
      pos - left * size * 1.5f + up * size,
  };
  DrawLineStrip3D(strip1, vp, glm::vec3(0.0f, 1.0f, 0.0f));
  DrawLineStrip3D(strip2, vp, glm::vec3(0.0f, 1.0f, 0.0f));
  DrawArrow(pos, pos + forward * size, vp, glm::vec3(0.0f, 1.0f, 0.0f),
            size * 0.12f);
}

void DrawPointLight(glm::vec3 pos, glm::mat4 vp, float size) {
  DrawWireSphere(pos, vp, size);
}

void DrawCube(glm::vec3 position, glm::vec3 forward, glm::vec3 left,
              glm::vec3 up, glm::mat4 vp, glm::vec3 size, glm::vec3 color) {
  float fd = size.z, ld = size.x, ud = size.y;
  std::vector<glm::vec3> strip1{position,
                                position + left * ld,
                                position + left * ld + forward * fd,
                                position + forward * fd,
                                position,
                                position + up * ud,
                                position + forward * fd + up * ud,
                                position + forward * fd};
  std::vector<glm::vec3> strip2{position + left * ld + up * ud,
                                position + up * ud,
                                position + forward * fd + up * ud,
                                position + left * ld + forward * fd + up * ud,
                                position + left * ld + up * ud,
                                position + left * ld,
                                position + left * ld + forward * fd,
                                position + left * ld + forward * fd + up * ud};
  DrawLineStrip3D(strip1, vp, color);
  DrawLineStrip3D(strip2, vp, color);
}

void DrawWireSphere(glm::vec3 position, glm::mat4 vp, float radius,
                    glm::vec3 color) {
  static int segs = 32;
  glm::vec3 walkDir1 = glm::vec3(1.0f, 0.0f, 0.0f);
  glm::vec3 walkDir2 = glm::vec3(0.0f, 1.0f, 0.0f);
  glm::vec3 walkDir3 = glm::vec3(0.0f, 0.0f, 1.0f);
  std::vector<glm::vec3> strip1{position + walkDir1 * radius};
  std::vector<glm::vec3> strip2{position + walkDir2 * radius};
  std::vector<glm::vec3> strip3{position + walkDir3 * radius};
  float rotDegree = glm::radians(360.0f / segs);
  for (int i = 0; i < segs; ++i) {
    walkDir1 =
        glm::angleAxis(rotDegree * (i + 1), glm::vec3(0.0f, 1.0f, 0.0f)) *
        glm::vec3(1.0f, 0.0f, 0.0f);
    walkDir2 =
        glm::angleAxis(rotDegree * (i + 1), glm::vec3(0.0f, 0.0f, 1.0f)) *
        glm::vec3(0.0f, 1.0f, 0.0f);
    walkDir3 =
        glm::angleAxis(rotDegree * (i + 1), glm::vec3(1.0f, 0.0f, 0.0f)) *
        glm::vec3(0.0f, 0.0f, 1.0f);
    strip1.push_back(position + walkDir1 * radius);
    strip2.push_back(position + walkDir2 * radius);
    strip3.push_back(position + walkDir3 * radius);
  }
  DrawLineStrip3D(strip1, vp, color);
  DrawLineStrip3D(strip2, vp, color);
  DrawLineStrip3D(strip3, vp, color);
}

void DrawCamera(glm::vec3 forward, glm::vec3 up, glm::vec3 left, glm::vec3 pos,
                glm::mat4 vp, float fovY, float aspect, float size) {
  // std::vector<glm::vec3> strip {

  // };
}

void DrawAABB(glm::vec3 min, glm::vec3 max, glm::mat4 &vp, glm::vec3 color) {
  DrawCube(min, glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 0.0f, 0.0f),
           glm::vec3(0.0f, 1.0f, 0.0f), vp, max - min, color);
}

}; // namespace VisUtils

}; // namespace aEngine