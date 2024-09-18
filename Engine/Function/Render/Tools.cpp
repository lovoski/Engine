#include "Function/Render/Tools.hpp"
#include "Function/AssetsLoader.hpp"
#include "Function/Render/Mesh.hpp"
#include "Function/Render/Shader.hpp"

namespace aEngine {

namespace Render {

std::string Equirectangular2CubemapVS = R"(
#version 330 core
layout (location = 0) in vec4 aPos;

out vec3 localPos;

uniform mat4 view;
uniform mat4 projection;

void main() {
  localPos = aPos.xyz;
  gl_Position = projection * view * aPos;
}
)";
std::string Equirectangular2CubemapFS = R"(
#version 330 core
out vec4 FragColor;
in vec3 localPos;

uniform sampler2D equirectangularMap;

const vec2 invAtan = vec2(0.1591, 0.3183);
vec2 SampleSphericalMap(vec3 v) {
  vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
  uv *= invAtan;
  uv += 0.5;
  return uv;
}

void main() {
  vec2 uv = SampleSphericalMap(normalize(localPos)); // make sure to normalize localPos
  vec3 color = texture(equirectangularMap, uv).rgb;
  FragColor = vec4(color, 1.0);
}
)";
void HDRToCubeMap(unsigned int &hdr, unsigned int &cubemap,
                  unsigned int cubeDimension) {
  static Shader shader;
  static VAO vao;
  static bool initialized = false;
  static int indices;
  if (!initialized) {
    vao.Bind();
    shader.LoadAndRecompileShaderSource(Equirectangular2CubemapVS,
                                        Equirectangular2CubemapFS);
    auto cube = Loader.GetMesh("::cubePrimitive", "");
    indices = cube->indices.size();
    cube->vbo.BindAs(GL_ARRAY_BUFFER);
    cube->ebo.BindAs(GL_ELEMENT_ARRAY_BUFFER);
    vao.LinkAttrib(cube->vbo, 0, 4, GL_FLOAT, sizeof(Vertex), (void *)0);
    vao.Unbind();
    cube->vbo.UnbindAs(GL_ARRAY_BUFFER);
    cube->ebo.UnbindAs(GL_ELEMENT_ARRAY_BUFFER);
    initialized = true;
  }
  // store previous states
  int previousFBO, previousViewport[4];
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &previousFBO);
  glGetIntegerv(GL_VIEWPORT, previousViewport);
  // start rendering the hdr to cubemap
  unsigned int captureFBO, captureRBO;
  glGenFramebuffers(1, &captureFBO);
  glGenRenderbuffers(1, &captureRBO);
  glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, cubeDimension,
                        cubeDimension);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, captureRBO);
  glGenTextures(1, &cubemap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
  for (int i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F,
                 cubeDimension, cubeDimension, 0, GL_RGB, GL_FLOAT, nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glm::mat4 captureProjection =
      glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
  glm::mat4 captureViews[] = {
      glm::lookAt(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f)),
      glm::lookAt(glm::vec3(0.0f), glm::vec3(-1.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f)),
      glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
                  glm::vec3(0.0f, 0.0f, 1.0f)),
      glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, -1.0f, 0.0f),
                  glm::vec3(0.0f, 0.0f, -1.0f)),
      glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 1.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f)),
      glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f),
                  glm::vec3(0.0f, -1.0f, 0.0f)),
  };
  shader.Use();
  shader.SetTexture2D(hdr, "equirectangularMap", 0);
  shader.SetMat4("projection", captureProjection);
  glViewport(0, 0, cubeDimension, cubeDimension);
  for (int i = 0; i < 6; ++i) {
    shader.SetMat4("view", captureViews[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    vao.Bind();
    glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_INT, 0);
    vao.Unbind();
  }
  // restore framebuffer and viewport after the render completes
  glFinish();
  glDeleteFramebuffers(1, &captureFBO);
  glDeleteRenderbuffers(1, &captureRBO);
  glBindFramebuffer(GL_FRAMEBUFFER, previousFBO);
  glViewport(previousViewport[0], previousViewport[1], previousViewport[2],
             previousViewport[3]);
}

const std::string skyboxVS = R"(
#version 330 core
layout (location = 0) in vec4 aPos;

out vec3 texCoord;

uniform mat4 VP;

void main() {
  texCoord = aPos.xyz;
  gl_Position = VP * aPos;
}  
)";
const std::string skyboxFS = R"(
#version 330 core
out vec4 FragColor;

in vec3 texCoord;
uniform samplerCube skybox;

void main() {
  FragColor = texture(skybox, texCoord);
}
)";
void RenderEnvironmentMap(unsigned int handle, glm::mat4 &vp) {
  static VAO vao;
  static Shader skyboxShader;
  static bool initialized = false;
  static unsigned int indices;
  if (!initialized) {
    vao.Bind();
    skyboxShader.LoadAndRecompileShaderSource(skyboxVS, skyboxFS);
    auto cube = Loader.GetMesh("::cubePrimitive", "");
    cube->vbo.BindAs(GL_ARRAY_BUFFER);
    cube->ebo.BindAs(GL_ELEMENT_ARRAY_BUFFER);
    indices = cube->indices.size();
    vao.LinkAttrib(cube->vbo, 0, 4, GL_FLOAT, sizeof(Vertex), (void *)0);
    vao.Unbind();
    cube->vbo.UnbindAs(GL_ARRAY_BUFFER);
    cube->ebo.UnbindAs(GL_ELEMENT_ARRAY_BUFFER);
    initialized = true;
  }
  glDepthMask(GL_FALSE);
  skyboxShader.Use();
  skyboxShader.SetMat4("VP", vp);
  vao.Bind();
  glBindTexture(GL_TEXTURE_CUBE_MAP, handle);
  glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_INT, 0);
  vao.Unbind();
  glDepthMask(GL_TRUE);
}

}; // namespace Render

}; // namespace aEngine