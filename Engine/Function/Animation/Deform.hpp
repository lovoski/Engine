#pragma once

#include "Entity.hpp"
#include "Global.hpp"
#include "Scene.hpp"

#include "Function/General/ComputeShader.hpp"
#include "Function/Render/Buffers.hpp"
#include "Function/Render/Mesh.hpp"

#include "Component/Animator.hpp"

namespace aEngine {

void DeformSkinnedMesh(Animator *animator, Render::Buffer &inputVBO,
                       unsigned int elementNum, Render::Buffer &targetVBO,
                       Render::Buffer &matrices);

}; // namespace aEngine