#pragma once

#include "Entity.hpp"
#include "Global.hpp"
#include "Scene.hpp"

#include "Function/General/ComputeShader.hpp"
#include "Function/Render/Buffers.hpp"
#include "Function/Render/Mesh.hpp"

#include "Component/Animator.hpp"

namespace aEngine {

void DeformBlendSkinnedMesh(Animator *animator, Render::Buffer &inputVBO,
                            unsigned int elementNum, Render::Buffer &targetVBO,
                            Render::Buffer &matrices,
                            int numBlendShapes,
                            Render::Buffer &blendShapeWeightsBuffer,
                            Render::Buffer &blendShapeOffsetBuffer);

void DeformSkinnedMesh(Animator *animator, Render::Buffer &inputVBO,
                       unsigned int elementNum, Render::Buffer &targetVBO,
                       Render::Buffer &matrices);

}; // namespace aEngine