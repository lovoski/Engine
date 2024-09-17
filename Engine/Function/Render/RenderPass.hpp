#pragma once

#include "Component/Camera.hpp"
#include "Component/Light.hpp"
#include "Function/AssetsType.hpp"
#include "Function/Render/Buffers.hpp"
#include "Function/Render/Shader.hpp"
#include "Global.hpp"

namespace aEngine {

namespace Render {

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

  Shader *GetShader() { return shader.get(); }

  // Setup lights in the environment automatically,
  // create variables with predefined names.
  // The parameter `bindingPoint` is the binding point of
  // light buffer as a SSBO.
  void SetupLights(Buffer &lightsBuffer,
                   std::shared_ptr<SkyLight> skyLight = nullptr,
                   int bindingPoint = 0);

  // don't override this function in custom pass
  void DrawInspectorGUIInternal();

  // This function will get called before the rendering
  void BeforePassInternal(glm::mat4 &model, glm::mat4 &view,
                          glm::mat4 &projection, glm::vec3 &viewDir,
                          bool receiveShadow);
  // This function will get called after the rendering
  virtual void FinishPass() {}
  virtual std::string getInspectorWindowName();

protected:
  int idCounter = 0;

  std::shared_ptr<Shader> shader = nullptr;

  // To create custom material, override this function,
  // pass custom variables to the shader, this function is called
  // at the end of the DrawInspectorGUI function
  virtual void DrawInspectorGUI() {}

  // Overload this function in custom materials
  // Setup variables to member `shader`
  // Activate textures with the helper function above
  virtual void BeforePass() {}
};

extern const std::string basicVS, basicFS, basicGS;
class Basic : public BasePass {
public:
  Basic();

  float Ambient = 0.03f;
  glm::vec3 Albedo = glm::vec3(1.0f);

  bool viewNormal = false;

  bool withWireframe = false;
  float WireframeWidth = 0.5f, WireframeSmooth = 1.0f;
  glm::vec3 WireframeColor = glm::vec3(0.0f);

protected:
  void BeforePass() override;
  void DrawInspectorGUI() override;

  std::string getInspectorWindowName() override;
};

extern const std::string outlineVS, outlineFS;
class OutlinePass : public BasePass {
public:
  OutlinePass();

  float OutlineWidth = 0.02f;
  // interpolate between OutlineColor and OutlineColorMap
  float OutlineWeight = 0.0f;
  glm::vec3 OutlineColor = glm::vec3(0.0f);

  Texture OutlineColorMap;

  void FinishPass() override;
  std::string getInspectorWindowName() override;

protected:
  void BeforePass() override;
  void DrawInspectorGUI() override;
};

extern const std::string wireframeVS, wireframeFS;
class WireFramePass : public BasePass {
public:
  WireFramePass();

  float wireframeOffset = 0.001f;
  glm::vec3 wireFrameColor = glm::vec3(1.0f);

  void FinishPass() override;
  std::string getInspectorWindowName() override;

private:
  void BeforePass() override;
  void DrawInspectorGUI() override;
};

extern const std::string GBVMainVS, GBVMainFS;
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

  std::string getInspectorWindowName() override;
  void FinishPass() override;

private:
  void BeforePass() override;
  void DrawInspectorGUI() override;
};

extern const std::string pbrVS, pbrFS;
class PBRPass : public BasePass {
public:
  PBRPass();

  float RoughnessFactor = 1.0f;
  float MetallicFactor = 1.0f;
  float AOFactor = 1.0f;
  float Ambient = 0.03f;
  glm::vec3 AlbedoFactor = glm::vec3(1.0f);
  glm::vec3 F0 = glm::vec3(0.04f);
  bool WithNormalMap = false;
  Texture Roughness, Metallic, AO, Albedo, Normal;

  std::string getInspectorWindowName() override;
  void FinishPass() override;

private:
  void BeforePass() override;
  void DrawInspectorGUI() override;
};

}; // namespace Render

}; // namespace aEngine
