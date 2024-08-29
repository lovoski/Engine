#include "Component/Light.hpp"
#include "Entity.hpp"
#include "Scene.hpp"

namespace aEngine {

Light::Light() {
  // Store current framebuffer
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
  // config the shadow map texture
  glGenTextures(1, &ShadowMap);
  glBindTexture(GL_TEXTURE_2D, ShadowMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, ShadowMapWidth,
               ShadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // config the framebuffer
  glGenFramebuffers(1, &ShadowFBO);
  glBindFramebuffer(GL_FRAMEBUFFER, ShadowFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         ShadowMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  // restore framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
}
Light::~Light() {
  glDeleteFramebuffers(1, &ShadowFBO);
  glDeleteTextures(1, &ShadowMap);
}

void Light::StartShadow() {
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFBO);
  glGetIntegerv(GL_VIEWPORT, viewport);
  glViewport(0, 0, ShadowMapWidth, ShadowMapHeight);
  glBindFramebuffer(GL_FRAMEBUFFER, ShadowFBO);
  glClear(GL_DEPTH_BUFFER_BIT);
}

void Light::EndShadow() {
  glBindFramebuffer(GL_FRAMEBUFFER, currentFBO);
  glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

glm::mat4 Light::GetShadowSpaceOrthoMatrix() {
  auto projMat =
      glm::ortho(-ShadowOrthoW, ShadowOrthoW, -ShadowOrthoH, ShadowOrthoH, ShadowZNear, ShadowZFar);
  auto entity = GWORLD.EntityFromID(entityID);
  auto viewMat =
      glm::lookAt(entity->Position(), entity->Position() + entity->LocalForward,
                  entity->LocalUp);
  return projMat * viewMat;
}

void ModifyLightColor(glm::vec3 &lightColor) {
  float color[3] = {lightColor.x, lightColor.y, lightColor.z};
  if (ImGui::ColorEdit3("Color", color)) {
    lightColor.x = color[0];
    lightColor.y = color[1];
    lightColor.z = color[2];
  }
}

void Light::DrawInspectorGUI() {
  if (ImGui::TreeNode("Base Light")) {
    std::vector<const char *> comboItems = {"Directional light", "Point light"};
    static int baseLightGUIComboItemIndex = 0;
    ImGui::Combo("Light Type", &baseLightGUIComboItemIndex, comboItems.data(),
                 comboItems.size());

    ModifyLightColor(lightColor);

    ImGui::TreePop();
  }
}

}; // namespace aEngine