#pragma once

#include "ecs/ecs.hpp"
#include "ecs/components/Camera.hpp"
#include "ecs/components/Material.hpp"
#include "ecs/systems/render/Mesh.hpp"
#include "ecs/components/Lights.hpp"

class MeshRenderer : public ECS::BaseComponent {
  SerializableType(MeshRenderer);
public:
  MeshRenderer() {}
  MeshRenderer(Graphics::Mesh *mesh) : meshData(mesh) {}
  ~MeshRenderer() {}

  void Render(mat4 projMat, mat4 viewMat, Transform *camera, Transform *object, BaseMaterial *material, vector<BaseLight> &lights) {
    Resource::Shader *shader = material->GetShader();
    shader->Use();
    shader->SetVec3("ViewDir", -camera->LocalForward);
    shader->SetMat4("projection", projMat);
    shader->SetMat4("view", viewMat);
    shader->SetMat4("model", object->GetModelMatrix());

    material->SetBaseVariables();
    material->SetBaseLights(lights);

    meshData->Draw(*shader);
  }

  void SetMeshData(Graphics::Mesh *mesh) {
    meshData = mesh;
  }

  Graphics::Mesh *meshData = nullptr;
};