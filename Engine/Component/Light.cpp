#include "Component/Light.hpp"
#include "Entity.hpp"
#include "Scene.hpp"

#include "Function/AssetsLoader.hpp"
#include "Function/AssetsType.hpp"
#include "Function/GUI/Helpers.hpp"

#include "Function/Render/Tools.hpp"

namespace aEngine {

// -------------- Directional Light --------------

DirectionalLight::DirectionalLight(EntityID id) : Light(id) {
  glGenFramebuffers(1, &ShadowFBO);
  ResizeShadowMap(ShadowMapWidth, ShadowMapHeight);
}
DirectionalLight::~DirectionalLight() {
  GLint currentlyBoundFBO;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentlyBoundFBO);

  if (currentlyBoundFBO == ShadowFBO) {
    // If the shadow FBO is currently bound, unbind it before deletion
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    LOG_F(WARNING, "Shadow framebuffer is in use, unbind before delete");
  }

  glDeleteTextures(1, &ShadowMap);
  glDeleteFramebuffers(1, &ShadowFBO);
}

void DirectionalLight::StartShadow() {
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
  glGetIntegerv(GL_VIEWPORT, viewport);
  glViewport(0, 0, ShadowMapWidth, ShadowMapHeight);
  glBindFramebuffer(GL_FRAMEBUFFER, ShadowFBO);
  glClear(GL_DEPTH_BUFFER_BIT);
}

void DirectionalLight::EndShadow() {
  glFinish(); // don't return until previous render finishes
  glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
  glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void DirectionalLight::ResizeShadowMap(unsigned int width,
                                       unsigned int height) {
  ShadowMapWidth = width;
  ShadowMapHeight = height;
  if (glIsTexture(ShadowMap))
    glDeleteTextures(1, &ShadowMap);
  glGenTextures(1, &ShadowMap);
  // Store current framebuffer
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
  // config the shadow map texture
  glBindTexture(GL_TEXTURE_2D, ShadowMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, ShadowMapWidth,
               ShadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  GLfloat borderColor[] = {1.0, 1.0, 1.0, 1.0};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  // config the framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, ShadowFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         ShadowMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  // restore framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
  glBindTexture(GL_TEXTURE_2D, 0);
}

glm::mat4 DirectionalLight::GetLightSpaceMatrix() {
  auto projMat = glm::ortho(-ShadowOrthoW * 0.5f, ShadowOrthoW * 0.5f,
                            -ShadowOrthoH * 0.5f, ShadowOrthoH * 0.5f,
                            ShadowZNear, ShadowZFar);
  auto entity = GWORLD.EntityFromID(entityID);
  auto viewMat =
      glm::lookAt(entity->Position(), entity->Position() + entity->LocalForward,
                  entity->LocalUp);
  return projMat * viewMat;
}

void DirectionalLight::DrawInspectorGUI() {
  ImGui::Checkbox("Enable", &enable);
  ImGui::Separator();
  if (!enable)
    ImGui::BeginDisabled();
  GUIUtils::ColorEdit3("Color", LightColor);
  ImGui::MenuItem("Shadow", nullptr, nullptr, false);
  ImGui::MenuItem("Shadow Frustom", nullptr, nullptr, false);
  ImGui::Checkbox("Show Frustom", &ShowShadowFrustom);
  ImGui::SliderFloat("Width", &ShadowOrthoW, 0.0f, 100.0f);
  ImGui::SliderFloat("Height", &ShadowOrthoH, 0.0f, 100.0f);
  ImGui::DragFloat("Near", &ShadowZNear, 0.01f, 0.0f, 10.0f);
  ImGui::DragFloat("Far", &ShadowZFar, 1, 10.0f, 200.0f);
  if (!enable)
    ImGui::EndDisabled();
}

// -------------- Point Light --------------

PointLight::PointLight(EntityID id) : Light(id) {}
PointLight::~PointLight() {}

void PointLight::DrawInspectorGUI() {
  ImGui::Checkbox("Enable", &enable);
  ImGui::Separator();
  if (!enable)
    ImGui::BeginDisabled();
  GUIUtils::ColorEdit3("Color", LightColor);
  ImGui::DragFloat("Radius", &LightRadius, 0.1f, 0.0f, 10000.0f);
  if (!enable)
    ImGui::EndDisabled();
}

// -------------- Sky Light --------------

EnvironmentLight::EnvironmentLight(EntityID id) : Light(id) {
  // initilaize all faces
  for (int i = 0; i < 6; ++i)
    faces[i] = *Loader.GetTexture("::null_texture");
  hdr = *Loader.GetTexture("::null_texture");
  // create the cubemap by default
  createCubeMapFromImages();
}

EnvironmentLight::~EnvironmentLight() {}

void EnvironmentLight::DrawInspectorGUI() {
  ImGui::Checkbox("Enable", &enable);
  ImGui::Separator();
  if (!enable)
    ImGui::BeginDisabled();
  if (ImGui::TreeNode("From Images")) {
    if (ImGui::Button("Build Cubemap", {-1, 30}))
      createCubeMapFromImages();
    static const char *faceLabels[6] = {"POS X", "NEG X", "POS Y",
                                        "NEG Y", "POS Z", "NEG Z"};
    for (int i = 0; i < 6; ++i)
      GUIUtils::DragableTextureTarget(faceLabels[i], faces[i], false);
    ImGui::TreePop();
  }
  if (ImGui::TreeNode("From HDR")) {
    if (ImGui::Button("Build Cubemap", {-1, 30}))
      createCubeMapFromHDR();
    GUIUtils::DragableTextureTarget("HDR Image", hdr);
    ImGui::TreePop();
  }
  if (!enable)
    ImGui::EndDisabled();
}

void EnvironmentLight::createCubeMapFromHDR() {
  Render::HDRToCubeMap(hdr.id, CubeMap);
  Render::ConvoluteCubeMap(CubeMap, Irradiance);
}

void EnvironmentLight::createCubeMapFromImages() {
  static Texture pureBlack = *Loader.GetTexture("::black_texture");
  static const GLenum cubemapFaces[6] = {
      GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
      GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
      GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z};

  glGenTextures(1, &CubeMap);
  glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMap);

  std::vector<unsigned char *> textureData(6);
  // all textures of a cubemap must have uniformed size
  int w = width, h = height;
  for (int i = 0; i < 6; i++) {
    // Bind the existing texture
    unsigned int id = faces[i].id;
    if (faces[i].path == "::null_texture")
      id = pureBlack.id;
    glBindTexture(GL_TEXTURE_2D, id);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
    textureData[i] = new unsigned char[w * h * 3];
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, textureData[i]);
  }

  for (unsigned int i = 0; i < 6; ++i) {
    glTexImage2D(cubemapFaces[i], 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE,
                 textureData[i]);
    delete[] textureData[i];
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

}; // namespace aEngine