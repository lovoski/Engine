#pragma once

#include "Global.hpp"
#include "Utils/AssetsType.hpp"
#include "Base/BaseComponent.hpp"

namespace aEngine {

struct Material : public aEngine::BaseComponent {
  std::vector<aEngine::Render::MaterialData*> passes;
};

};
