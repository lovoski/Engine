#pragma once

#include "Base/BaseComponent.hpp"
#include "Component/Light.hpp"
#include "Entity.hpp"
#include "Scene.hpp"
#include "Utils/AssetsType.hpp"
#include "Utils/AssetsLoader.hpp"
#include "Utils/Render/MaterialData.hpp"

namespace aEngine {

struct MeshRenderer : public aEngine::BaseComponent {
  MeshRenderer() {}
  MeshRenderer(aEngine::Render::Mesh *mesh) : meshData(mesh) {}
  ~MeshRenderer() {}

  void ForwardRender(glm::mat4 projMat, glm::mat4 viewMat, Entity *camera,
                     Entity *object, std::vector<Light> &lights);

  Json Serialize() override;
  void Deserialize(Json &json) override;

  void DrawInspectorGUI() override;

  // Add a render pass for this renderer, pass in the nullptr
  // to instantiate a new pass of the defualt type
  template <typename T>
  void AddPass(T *pass, std::string identifier) {
    if (pass == nullptr) {
      // create default material
      passes.push_back(Loader.InstatiateMaterial<T>(identifier));
    } else {
      passes.push_back(pass);
    }
  }

  Render::Mesh *meshData = nullptr;

  std::vector<Render::BaseMaterial *> passes;
};

}; // namespace aEngine