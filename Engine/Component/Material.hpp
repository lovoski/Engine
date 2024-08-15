#pragma once

#include "Base/BaseComponent.hpp"
#include "Global.hpp"
#include "Utils/AssetsLoader.hpp"
#include "Utils/AssetsType.hpp"
#include "Utils/Render/MaterialData.hpp"

namespace aEngine {

struct Material : public aEngine::BaseComponent {
  std::vector<aEngine::Render::MaterialData *> passes;

  Material() {
    // create a default basic diffuse render pass
    passes.push_back(Loader.GetMaterial("::base"));
  }

  void DrawInspectorGUI() override;
};

}; // namespace aEngine
