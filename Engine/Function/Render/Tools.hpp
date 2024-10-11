#pragma once

#include "Function/AssetsType.hpp"
#include "Function/Render/Shader.hpp"
#include "Global.hpp"

namespace aEngine {

namespace Render {

void HDRToCubeMap(unsigned int &hdr, unsigned int &cubemap,
                  unsigned int cubeDimension = 512);

void RenderEnvironmentMap(unsigned int skybox, glm::mat4 &vp);

// Perform convolution over the source cubemap to get a target cubemap
// as irradiance map for pbr rendering.
void ConvoluteCubeMap(unsigned int &source, unsigned int &target);

// Render a texture as the output of a shader.
// Default variables:
// VS:
//   `uniform mat4 Model;`
//   `uniform mat4 View;`
//   `uniform mat4 Projection;`
// When using the `defaultProgramTextureVS` shader source, a texCoord variable
// will be setup for fragment shader, (0,0) is the top-left corner, (1, 1) is
// the bottom-right corner.
// Use the function:
// `ImGui::Image(target, size, {0, 1}, {1, 0});`
// to properly display the rendered texture.
void ProgramTexture(Shader &shader, unsigned int &target,
                    unsigned int width = 1024, unsigned int height = 1024);

// Default program texture vertex shader, pass `vec2 texCoord` to fragment
// shader.
extern const std::string defaultProgramTextureVS;

}; // namespace Render

}; // namespace aEngine