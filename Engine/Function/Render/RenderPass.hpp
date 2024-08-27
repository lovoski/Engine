#pragma once

#include "Component/Camera.hpp"
#include "Component/Light.hpp"
#include "Global.hpp"
#include "Function/AssetsType.hpp"
#include "Function/Render/Shader.hpp"


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
class BasePass {
public:
  BasePass() {}
  ~BasePass() {}

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
  void SetupLights(std::vector<std::shared_ptr<Light>> &lights);

  // To create custom material, override this function,
  // pass custom variables to the shader
  // This function is called by default at the end of SetupPass
  virtual void DrawInspectorGUI() { drawInspectorGUIDefault(); }

  // This function will get called before the rendering
  void SetupPass(glm::mat4 &model, glm::mat4 &view,
                           glm::mat4 &projection, glm::vec3 &viewDir);
  // This function will get called after the rendering
  virtual void FinishPass() {}

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

// the default diffuse shader
const std::string diffuseVS = R"(
#version 460 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aNormal;
layout (location = 2) in vec2 aTexCoord;
out vec3 normal;
// transform point from model space to world space
uniform mat4 ModelToWorldPoint;
// transform vector from model space to world space
uniform mat3 ModelToWorldDir;
// transform world space to camera space
uniform mat4 View;
// transform camera space to screen
uniform mat4 Projection;
void main() {
  normal = ModelToWorldDir * vec3(aNormal);
  gl_Position = Projection * View * ModelToWorldPoint * aPos;
}
)";

const std::string diffuseFS = R"(
#version 430 core
uniform vec3 Albedo;
uniform vec3 dLightDir0;
uniform vec3 dLightColor0;
uniform float Ambient;
in vec3 normal;
out vec4 FragColor;
void main() {
  // ambient
  vec3 ambient = Ambient * dLightColor0;
  // diffuse
  vec3 Normal = normalize(normal);
  vec3 LightDir = -dLightDir0;
  float lambert = (dot(Normal, LightDir) + 1.0) * 0.5;
  vec3 diffuse = lambert * dLightColor0 * Albedo;
  vec3 result = ambient + diffuse;
  FragColor = vec4(result, 1.0);
}
)";

class DiffuseMaterial : public BasePass {
public:
  DiffuseMaterial();

  float Ambient = 0.1f;
  glm::vec3 Albedo = glm::vec3(1.0f);

  void DrawInspectorGUI() override;

  void Serialize(Json &json) override;
  void Deserialize(Json &json) override;

protected:

  void setupCustomVariables() override;

  std::string getMaterialTypeName() override;
};

}; // namespace Render

}; // namespace aEngine
