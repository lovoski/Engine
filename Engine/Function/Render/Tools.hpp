#pragma once

#include "Global.hpp"

#include "Function/AssetsType.hpp"

namespace aEngine {

namespace Render {

void HDRToCubeMap(unsigned int &hdr, unsigned int &cubemap,
                  unsigned int cubeDimension = 512);

void RenderEnvironmentMap(unsigned int skybox, glm::mat4 &vp);

}; // namespace Render

}; // namespace aEngine