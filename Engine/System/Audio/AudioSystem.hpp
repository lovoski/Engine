#pragma once

#include "Base/BaseSystem.hpp"
#include "Global.hpp"

namespace aEngine {

class AudioSystem : public BaseSystem {
public:
  AudioSystem();
  ~AudioSystem();

  void PreUpdate(float dt) override;
  void Update(float dt) override;

  void Reset() override {}

  void DebugRender() override;

  template <typename Archive>
  void serialize(Archive &ar) {
    ar(cereal::base_class<BaseSystem>(this));
  }

private:
  ma_uint32 playbackDeviceCount = 0, captureDeviceCount = 0;
  ma_device_info *playbackDeviceInfo = nullptr, *captureDeviceInfo = nullptr;
  void queryAviableDevice();
};

}; // namespace aEngine