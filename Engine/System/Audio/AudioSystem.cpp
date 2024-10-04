#include "System/Audio/AudioSystem.hpp"

namespace aEngine {

AudioSystem::AudioSystem() { queryAviableDevice(); }

AudioSystem::~AudioSystem() {}

void AudioSystem::PreUpdate(float dt) {}

void AudioSystem::Update(float dt) {}

void AudioSystem::Render() {}

void AudioSystem::queryAviableDevice() {
  ma_result result;
  ma_context context;
  result = ma_context_init(NULL, 0, NULL, &context);
  if (result != MA_SUCCESS) {
    LOG_F(ERROR, "failed to initialize context");
    return;
  }
  result = ma_context_get_devices(&context, &playbackDeviceInfo,
                                  &playbackDeviceCount, &captureDeviceInfo,
                                  &captureDeviceCount);
  if (result != MA_SUCCESS) {
    LOG_F(ERROR, "failed to query devices");
    ma_context_uninit(&context);
    return;
  }
}

}; // namespace aEngine

REGISTER_SYSTEM(aEngine, AudioSystem)