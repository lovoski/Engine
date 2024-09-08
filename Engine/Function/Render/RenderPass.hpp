#pragma once

#include "Component/Camera.hpp"
#include "Component/Light.hpp"
#include "Function/AssetsType.hpp"
#include "Function/Render/Buffers.hpp"
#include "Function/Render/Shader.hpp"
#include "Global.hpp"

namespace aEngine {

namespace Render {

// Activate the texture at a opengl bind point, if the texture is ::null_texture,
// pure white texture will be activated instead.
bool ActivateTexture2D(Texture &texture, Shader *shader, std::string name,
                       int slot);

// Each material data should maintain an instance shader
// If the shader specified failed to load, a default error shader should be
// applied The material data should be able to represent a complete render pass
// In principle, each material should handle the shader loading at its
// constructor.
class BasePass {
public:
  BasePass() {}
  ~BasePass() {}

  std::string identifier;
  std::string path;
  bool Enabled = true;

  // force reload, compile and link the shader program
  void SetShader(Shader *s) {
    if (s == nullptr) {
      LOG_F(ERROR, "can\'t set shader to nullptr");
    } else {
      shader = s;
    }
  }

  Shader *GetShader() { return shader; }

  // Setup lights in the environment automatically,
  // create variables with predefined names
  void SetupLights(Buffer &lightsBuffer, int bindingPoint = 0);

  // don't override this function in custom pass
  void DrawInspectorGUI();

  // This function will get called before the rendering
  void SetupPass(glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection,
                 glm::vec3 &viewDir, bool receiveShadow);
  // This function will get called after the rendering
  virtual void FinishPass() {}
  virtual std::string GetMaterialTypeName();

protected:
  int idCounter = 0;

  Shader *shader = nullptr;

  // To create custom material, override this function,
  // pass custom variables to the shader, this function is called
  // at the end of the DrawInspectorGUI function
  virtual void drawCustomInspectorGUI() {}

  // Overload this function in custom materials
  // Setup variables to member `shader`
  // Activate textures with the helper function above
  virtual void additionalSetup() {}
};

class Diffuse : public BasePass {
public:
  Diffuse();

  float Ambient = 0.1f;
  glm::vec3 Albedo = glm::vec3(1.0f);

protected:
  void additionalSetup() override;
  void drawCustomInspectorGUI() override;

  std::string GetMaterialTypeName() override;
};

class OutlinePass : public BasePass {
public:
  OutlinePass();

  float OutlineWidth = 0.02f, OutlineWeight = 1.0f;
  glm::vec3 OutlineColor = glm::vec3(0.0f);

  void FinishPass() override;
  std::string GetMaterialTypeName() override;

protected:
  void additionalSetup() override;
  void drawCustomInspectorGUI() override;
};

class GBVMainPass : public BasePass {
public:
  GBVMainPass();

  // texture maps
  Texture base, ILM, SSS;

  // ramp
  float firstRampStart = 0.2, firstRampStop = 0.22;
  float rampOffset = 1.0f, rampShadowWeight = 1.0f;

  // rim light
  glm::vec3 rimLightColor = glm::vec3(0.8f);
  float rimLightWidth = 0.063f;
  float rimLightSmooth = 0.01f;

  // specular
  int specularGloss = 20;
  float specularWeight = 0.0f;

  // details
  float detailWeight = 0.3f;
  float innerLineWeight = 1.0f;
  Texture detail;

  std::string GetMaterialTypeName() override;
  void FinishPass() override;

private:
  void additionalSetup() override;
  void drawCustomInspectorGUI() override;
};

}; // namespace Render

}; // namespace aEngine
