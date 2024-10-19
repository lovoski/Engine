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
  MeshRenderer() : BaseComponent(0) {}
  MeshRenderer(EntityID id);
  ~MeshRenderer();

  void ForwardRender(std::shared_ptr<Mesh> mesh, glm::mat4 projMat,
                     glm::mat4 viewMat, Entity *camera, Entity *object,
                     Render::Buffer &lightsBuffer,
                     std::shared_ptr<EnvironmentLight> skyLight = nullptr);

  // Setup `Model` shader variable for the shader, draw the mesh
  void DrawMeshShadowPass(Render::Shader &shader, std::shared_ptr<Mesh> mesh,
                          glm::mat4 modelMat);

  void DrawMesh(Render::Shader &shader, std::shared_ptr<Mesh> mesh);

  void DrawInspectorGUI() override;

  // Add a render pass for this renderer, pass in the nullptr
  // to instantiate a new pass of the defualt type
  template <typename T>
  void AddPass(std::shared_ptr<T> pass, std::string identifier) {
    static_assert((std::is_base_of<Render::BasePass, T>::value),
                  "Invalid template type for MeshRenderer");
    if (pass == nullptr) {
      // create default material
      passes.push_back(Loader.InstantiateMaterial<T>(identifier));
    } else {
      passes.push_back(pass);
    }
  }

  // Get the pass of desired type, returns nullptr if don't exist
  template <typename T> std::shared_ptr<T> GetPass() {
    static_assert((std::is_base_of<Render::BasePass, T>::value),
                  "Invalid template type for MeshRenderer");
    for (auto pass : passes) {
      if (typeid(*(pass.get())) == typeid(T))
        return pass;
    }
    return nullptr;
  }

  std::string getInspectorWindowName() override { return "Mesh Renderer"; }

  template <typename Archive> void save(Archive &ar) const {
    ar(CEREAL_NVP(entityID));
    std::vector<bool> enableStatus;
    std::vector<std::string> identifiers;
    for (auto &pass : passes) {
      enableStatus.push_back(pass->Enabled);
      identifiers.push_back(pass->identifier);
    }
    ar(castShadow, receiveShadow, passes, enableStatus, identifiers);
  }
  template <typename Archive> void load(Archive &ar) {
    ar(CEREAL_NVP(entityID));
    std::vector<bool> enableStatus;
    std::vector<std::string> identifiers;
    ar(castShadow, receiveShadow, passes, enableStatus, identifiers);
    for (int i = 0; i < passes.size(); ++i) {
      passes[i]->Enabled = enableStatus[i];
      passes[i]->identifier = identifiers[i];
    }
  }

  bool castShadow = true;
  bool receiveShadow = true;
  std::vector<std::shared_ptr<Render::BasePass>> passes;

private:
  void drawAppendPassPopup();

  template <typename T> void handleAppendPass(std::string identifier) {
    for (auto &pass : passes) {
      if (typeid(*(pass.get())) == typeid(T)) {
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