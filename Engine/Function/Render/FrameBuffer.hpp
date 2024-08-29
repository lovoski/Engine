#pragma once

namespace aEngine {

class FrameBuffer {
public:
  FrameBuffer(float width, float height);
  ~FrameBuffer();

  unsigned int GetFrameTexture();

  void RescaleFrameBuffer(float width, float height);

  void Bind() const;
  void Unbind() const;
private:
  unsigned int FBO, RBO, texture;
};

};
