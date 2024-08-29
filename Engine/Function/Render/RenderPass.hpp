#pragma once

#include "Component/Camera.hpp"
#include "Component/Light.hpp"
#include "Function/AssetsType.hpp"
#include "Function/Render/Shader.hpp"
#include "Global.hpp"

namespace aEngine {

namespace Render {

enum RENDER_QUEUE { OPAQUE, TRANSPARENT };

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

  // force reload, compile and link the shader program
  void SetShader(Shader *s) {
    if (s == nullptr) {
      LOG_F(ERROR, "can\'t set shader to nullptr");
    } else {
      shader = s;
    }
  }

  Shader *GetShader() { return shader; }

  virtual void Serialize(Json &json) { json["matType"] = "base"; }
  virtual void Deserialize(Json &json) {}

  // Setup lights in the environment automatically,
  // create variables with predefined names
  void SetupLights(std::vector<std::shared_ptr<Light>> &lights);

  // To create custom material, override this function,
  // pass custom variables to the shader
  // This function is called by default at the end of SetupPass
  virtual void DrawInspectorGUI() { drawInspectorGUIDefault(); }

  // This function will get called before the rendering
  void SetupPass(glm::mat4 &model, glm::mat4 &view, glm::mat4 &projection,
                 glm::vec3 &viewDir);
  // This function will get called after the rendering
  virtual void FinishPass() {}

protected:
  int idCounter = 0;

  Shader *shader = nullptr;

  void drawInspectorGUIDefault();

  // Overload this function in custom materials
  // Setup variables to member `shader`
  // Activate textures with the helper function above
  virtual void additionalSetup() {}
  virtual std::string getMaterialTypeName();
};

class Diffuse : public BasePass {
public:
  Diffuse();

  float Ambient = 0.1f;
  glm::vec3 Albedo = glm::vec3(1.0f);

  void DrawInspectorGUI() override;

  void Serialize(Json &json) override;
  void Deserialize(Json &json) override;

protected:
  void additionalSetup() override;

  std::string getMaterialTypeName() override;
};

class OutlinePass : public BasePass {
public:
  OutlinePass();

  float OutlineWidth = 0.02f, OutlineWeight = 1.0f;
  glm::vec3 OutlineColor = glm::vec3(0.0f);

  void DrawInspectorGUI() override;

  void FinishPass() override;

  void Serialize(Json &json) override;
  void Deserialize(Json &json) override;

protected:
  void additionalSetup() override;

  std::string getMaterialTypeName() override;
};

}; // namespace Render

}; // namespace aEngine
