#pragma once

#include "Base/BaseComponent.hpp"
#include "Component/Light.hpp"
#include "Component/Material.hpp"
#include "Entity.hpp"
#include "Scene.hpp"
#include "Utils/AssetsType.hpp"


namespace aEngine {

struct MeshRenderer : public aEngine::BaseComponent {
  MeshRenderer() {}
  MeshRenderer(aEngine::Render::Mesh *mesh) : meshData(mesh) {}
  ~MeshRenderer() {}

  void ForwardRender(glm::mat4 projMat, glm::mat4 viewMat, Entity *camera,
                     Entity *object, Material *material,
                     std::vector<Light> &lights);

  Json Serialize() override;
  void Deserialize(Json &json) override;

  Render::Mesh *meshData = nullptr;
};

}; // namespace aEngine