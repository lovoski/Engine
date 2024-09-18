#pragma once

#include "Global.hpp"

#include "Function/AssetsType.hpp"

namespace aEngine {

namespace Render {

void HDRToCubeMap(unsigned int &hdr, unsigned int &cubemap,
                  unsigned int cubeDimension = 512);

void RenderEnvironmentMap(unsigned int skybox, glm::mat4 &vp);

// Perform convolution over the source cubemap to get a target cubemap
// as irradiance map for pbr rendering.
void ConvoluteCubeMap(unsigned int &source, unsigned int &target);

}; // namespace Render

}; // namespace aEngine