#pragma once

#include "Component/Camera.hpp"
#include "Component/Light.hpp"
#include "Function/AssetsType.hpp"
#include "Function/Render/Buffers.hpp"
#include "Function/Render/Shader.hpp"
#include "Global.hpp"

// place this macro outside all namespace inside the source file where the
// render pass is defined.
#define REGISTER_RENDER_PASS(Namespace, RenderPassType)                        \
  CEREAL_REGISTER_TYPE(Namespace::RenderPassType);                             \
  CEREAL_REGISTER_POLYMORPHIC_RELATION(aEngine::Render::BasePass,              \
                                       Namespace::RenderPassType)

// IMPORTANT!!!
// as the BasePass class already have the serialize function, by default the
// serialization process should take place in serialize function only, if
// you use the (load, save) function pair instead, declare the macro
// `CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES` in the source of relative render pass
// to do disambiguation. Refer to `Outlin.cpp` for more details.
// Or you can use the following macro helper instead
#define REGISTER_RENDER_PASS_SL(Namespace, RenderPassType)                     \
  CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(Namespace::RenderPassType,                \
                                     cereal::specialization::member_load_save) \
  REGISTER_RENDER_PASS(Namespace, RenderPassType)

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
                   std::shared_ptr<EnvironmentLight> skyLight = nullptr,
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

  template <typename Archive> void serialize(Archive &ar) {}

protected:
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

  template <typename Archive> void serialize(Archive &ar) {
    ar(Enabled, Ambient, Albedo, viewNormal, withWireframe, WireframeWidth,
       WireframeSmooth, WireframeColor);
  }

  void FinishPass() override;

protected:
  void BeforePass() override;
  void DrawInspectorGUI() override;

  std::string getInspectorWindowName() override;
};

}; // namespace Render

}; // namespace aEngine
