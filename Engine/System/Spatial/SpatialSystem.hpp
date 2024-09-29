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

  template <typename Archive>
  void serialize(Archive &archive, const unsigned int version) {
    boost::serialization::base_object<BaseSystem>(*this);
  }

private:
};

}; // namespace aEngine