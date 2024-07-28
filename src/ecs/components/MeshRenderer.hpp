#pragma once

#include "ecs/ecs.hpp"
#include "ecs/components/Camera.hpp"
#include "ecs/components/Transform.hpp"
#include "ecs/components/Material.hpp"
#include "ecs/systems/render/Mesh.hpp"
#include "ecs/components/Lights.hpp"

class MeshRenderer : public ECS::BaseComponent {
public:
  MeshRenderer() {}
  MeshRenderer(Graphics::Mesh *mesh) : meshData(mesh) {}
  ~MeshRenderer() {}

  void Render(mat4 projMat, mat4 viewMat, Transform &transform, BaseMaterial *material, vector<BaseLight> &lights) {
    Resource::Shader *shader = material->GetShader();
    shader->Use();
    shader->SetMat4("projection", projMat);
    shader->SetMat4("view", viewMat);
    shader->SetMat4("model", transform.GetModelMatrix());
    material->SetFixedVariables();
    material->SetBaseLights(lights);
    material->ActivateTextures();
    meshData->Draw(*shader);
  }

  void SetMeshData(Graphics::Mesh *mesh) {
    meshData = mesh;
  }

  Graphics::Mesh *meshData = nullptr;
};