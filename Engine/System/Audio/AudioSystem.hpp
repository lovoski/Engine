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

  void Render();

  template <typename Archive>
  void serialize(Archive &archive, const unsigned int version) {
    boost::serialization::base_object<BaseSystem>(*this);
  }

private:
  ma_uint32 playbackDeviceCount = 0, captureDeviceCount = 0;
  ma_device_info *playbackDeviceInfo = nullptr, *captureDeviceInfo = nullptr;
  void queryAviableDevice();
};

}; // namespace aEngine