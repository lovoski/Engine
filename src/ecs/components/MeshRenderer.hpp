#pragma once

#include "ecs/ecs.hpp"
#include "ecs/components/Camera.hpp"
#include "ecs/components/Material.hpp"
#include "ecs/systems/render/Mesh.hpp"
#include "ecs/components/Lights.hpp"

#include "engine/Engine.hpp"

class MeshRenderer : public ECS::BaseComponent {
public:
  MeshRenderer() {}
  MeshRenderer(Graphics::Mesh *mesh) : meshData(mesh) {}
  ~MeshRenderer() {}

  void Render(mat4 projMat, mat4 viewMat, Transform *camera, Transform *object, Material *material, vector<Light> &lights) {
    // each materail could have multiple render passes in different render queues
    for (auto pass : material->passes) {
      auto shader = pass->GetShader();
      shader->Use();
      shader->SetVec3("ViewDir", -camera->LocalForward);
      shader->SetMat4("projection", projMat);
      shader->SetMat4("view", viewMat);
      shader->SetMat4("model", object->GetModelMatrix());
      pass->SetupVariables();
      pass->SetupLights(lights);
      meshData->Draw(*shader);
    }
  }

  void SetMeshData(Graphics::Mesh *mesh) {
    meshData = mesh;
  }

  void Serialize(Json &json) override {
    json["mesh"]["modelpath"] = meshData->modelPath;
    json["mesh"]["identifier"] = meshData->identifier;
  }

  void Deserialize(Json &json) override {
    string modelPath = json["mesh"]["modelpath"];
    string identifier = json["mesh"]["identifier"];
    meshData = Core.RManager.GetMesh(modelPath, identifier);
  }

  Graphics::Mesh *meshData = nullptr;
};