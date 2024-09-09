#pragma once

#include "Base/BaseComponent.hpp"
#include "Component/Light.hpp"
#include "Entity.hpp"
#include "Function/AssetsLoader.hpp"
#include "Function/AssetsType.hpp"
#include "Scene.hpp"

#include "Component/Mesh.hpp"

#include "Function/Render/Buffers.hpp"
#include "Function/Render/Mesh.hpp"
#include "Function/Render/RenderPass.hpp"

namespace aEngine {

struct MeshRenderer : public aEngine::BaseComponent {
  MeshRenderer(EntityID id);
  ~MeshRenderer();

  void ForwardRender(std::shared_ptr<Mesh> mesh, glm::mat4 projMat,
                     glm::mat4 viewMat, Entity *camera, Entity *object,
                     Render::Buffer &lightsBuffer);

  // Setup `Model` shader variable for the shader, draw the mesh
  void DrawMeshShadowPass(Render::Shader &shader, std::shared_ptr<Mesh> mesh,
                          glm::mat4 modelMat);

  void DrawMesh(Render::Shader &shader, std::shared_ptr<Mesh> mesh);

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

  bool castShadow = true;
  bool receiveShadow = true;
  std::vector<Render::BasePass *> passes;

private:
  void drawAppendPassPopup();

  template <typename T> void handleAppendPass(std::string identifier) {
    for (auto &pass : passes) {
      if (typeid(*pass) == typeid(T)) {
        // there's a pass of the same type
        // replace the original pass with a new one
        LOG_F(INFO, "replacing material %s with new %s",
              pass->identifier.c_str(), identifier.c_str());
        pass = Loader.InstantiateMaterial<T>(identifier);
        return;
      }
    }
    // no pass with the same type found
    AddPass<T>(nullptr, identifier);
  }
};

}; // namespace aEngine