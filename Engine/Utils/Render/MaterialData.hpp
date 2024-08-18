#pragma once

#include "Component/Camera.hpp"
#include "Component/Light.hpp"
#include "Global.hpp"
#include "Utils/AssetsType.hpp"
#include "Utils/Render/Shader.hpp"


namespace aEngine {

namespace Render {

enum RENDER_QUEUE {
  OPAQUE,
  TRANSPARENT
};

bool ActivateTexture2D(Texture &texture, Shader *shader, std::string name,
                       int slot);

// Each material data should maintain an instance shader
// If the shader specified failed to load, a default error shader should be
// applied The material data should be able to represent a complete render pass
// In principle, each material should handle the shader loading at its constructor
class BaseMaterial {
public:
  BaseMaterial() {}
  ~BaseMaterial() {}

  std::string identifier;
  std::string path;

  // force reload, compile and link the shader program
  void SetShader(Shader *s) {
    if (s == nullptr) {
      Console.Log("[error]: can't set shader to nullptr");
    } else {
      shader = s;
    }
  }

  Shader *GetShader() { return shader; }

  virtual void Serialize(Json &json) { json["matType"] = "base"; }
  virtual void Deserialize(Json &json) {}

  // Setup lights in the environment automatically,
  // create variables with predefined names
  void SetupLights(std::vector<Light> &lights);

  // To create custom material, override this function,
  // pass custom variables to the shader
  // This function is called by default at the end of SetVariables
  virtual void DrawInspectorGUI() { drawInspectorGUIDefault(); }

  // This function will get called in the renderer
  void SetVariables(glm::mat4 &model, glm::mat4 &view,
                           glm::mat4 &projection, glm::vec3 &viewDir);

protected:
  int idCounter = 0;

  Shader *shader = nullptr;

  void drawInspectorGUIDefault();

  // Overload this function in custom materials
  // Setup variables to member `shader`
  // Activate textures with the helper function above
  virtual void setupCustomVariables() {}
  virtual std::string getMaterialTypeName();
};

class DiffuseMaterial : public BaseMaterial {
public:
  DiffuseMaterial(bool deformable = false);

  float Ambient = 0.1f;
  glm::vec3 Albedo = glm::vec3(1.0f);

  void DrawInspectorGUI() override;

  void Serialize(Json &json) override;
  void Deserialize(Json &json) override;

protected:

  void setupCustomVariables() override;
};

}; // namespace Render

}; // namespace aEngine
