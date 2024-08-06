#pragma once

#include "Lights.hpp"
#include "ecs/ecs.hpp"
#include "resource/ResourceManager.hpp"
#include "resource/ResourceTypes.hpp"
#include "resource/Shader.hpp"
#include "resource/MaterialData.hpp"

#include "engine/Engine.hpp"

class Material : public ECS::BaseComponent {
public:
  Material() {
    // load the default material data
    // each material data has only one instance
    // modifing one instance would have effect on all references
    passes.push_back(Core.RManager.GetMaterialData("::base"));
  }
  ~Material() {
    passes.clear();
  }

  vector<MaterialData *> passes;

  void Serialize(Json &json) override {
    for (auto pass : passes) {
      json["passes"][pass->identifier] = pass->path;
    }
  }

  void Deserialize(Json &json) override {
    // clear the passes first
    // the free of material data will be handled by resource manager
    passes.clear();

    // reload all passes
    for (auto pass : json["passes"].items()) {
      auto ptr = Core.RManager.GetMaterialData(pass.value());
      if (ptr != nullptr) {
        // if the pass was successfully loaded
        passes.push_back(ptr);
      }
    }
  }

};