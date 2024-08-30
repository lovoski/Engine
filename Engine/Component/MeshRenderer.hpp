#pragma once

#include "Base/BaseComponent.hpp"
#include "Component/Light.hpp"
#include "Entity.hpp"
#include "Function/AssetsLoader.hpp"
#include "Function/AssetsType.hpp"
#include "Scene.hpp"

#include "Function/Render/Buffers.hpp"
#include "Function/Render/RenderPass.hpp"
#include "Function/Render/Mesh.hpp"

namespace aEngine {

struct MeshRenderer : public aEngine::BaseComponent {
  MeshRenderer(aEngine::Render::Mesh *mesh);
  ~MeshRenderer();

  void ForwardRender(glm::mat4 projMat, glm::mat4 viewMat, Entity *camera,
                     Entity *object, Render::Buffer &lightsBuffer);

  void DrawMesh(Render::Shader &shader);

  Json Serialize() override;
  void Deserialize(Json &json) override;

  void DrawInspectorGUI() override;

  // Add a render pass for this renderer, pass in the nullptr
  // to instantiate a new pass of the defualt type
  template <typename T> void AddPass(T *pass, std::string identifier) {
    if (pass == nullptr) {
      // create default material
      passes.push_back(Loader.InstantiateMaterial<T>(identifier));
    } else {
      passes.push_back(pass);
    }
  }

  Render::VAO vao;
  bool castShadow = true;
  bool receiveShadow = true;
  std::vector<glm::mat4> lightSpaceMatrices;
  Render::Mesh *meshData = nullptr;
  Render::Buffer *targetVBO = nullptr;
  std::vector<Render::BasePass *> passes;
};

}; // namespace aEngine