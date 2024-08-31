#pragma once

#include "Entity.hpp"
#include "Global.hpp"
#include "Scene.hpp"

#include "Function/General/ComputeShader.hpp"
#include "Function/Render/Buffers.hpp"
#include "Function/Render/Mesh.hpp"

#include "Component/Animator.hpp"

namespace aEngine {

void DeformSkinnedMesh(Render::Mesh *mesh, Animator *animator,
                       Render::Buffer &targetVBO, Render::Buffer &matrices);

}; // namespace aEngine