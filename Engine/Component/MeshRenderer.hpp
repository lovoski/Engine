#pragma once

#include "Base/BaseComponent.hpp"
#include "Component/Light.hpp"
#include "Entity.hpp"
#include "Function/AssetsLoader.hpp"
#include "Function/AssetsType.hpp"
#include "Scene.hpp"

#include "Function/Render/Buffers.hpp"
#include "Function/Render/Mesh.hpp"
#include "Function/Render/RenderPass.hpp"


namespace aEngine {

struct MeshRenderer : public aEngine::BaseComponent {
  MeshRenderer(EntityID id, aEngine::Render::Mesh *mesh);
  ~MeshRenderer();

  void ForwardRender(glm::mat4 projMat, glm::mat4 viewMat, Entity *camera,
                     Entity *object, Render::Buffer &lightsBuffer);

  // Setup `Model` shader variable for the shader, draw the mesh
  void DrawMeshShadowPass(Render::Shader &shader, glm::mat4 modelMat);

  void DrawMesh(Render::Shader &shader);

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

private:
  void drawAppendPassPopup();

  template<typename T>
  bool hasMaterial() {
    for (auto pass : passes) {
      if (typeid(*pass) == typeid(T))
        return true;
    }
    return false;
  }
};

}; // namespace aEngine