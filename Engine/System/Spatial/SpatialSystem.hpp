#pragma once

#include "Base/BaseSystem.hpp"
#include "Global.hpp"


namespace aEngine {

class SpatialSystem : public BaseSystem {
public:
  SpatialSystem();
  ~SpatialSystem();

  void Reset() override {}

  void PreUpdate(float dt) override;
  void Update(float dt) override;

  // Render debug utils related to geometry
  void Render();

private:
};

};