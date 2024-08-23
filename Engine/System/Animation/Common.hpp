#pragma once

#include "Global.hpp"
#include "Scene.hpp"
#include "Entity.hpp"

#include "Function/Render/Mesh.hpp"
#include "Function/Render/Buffers.hpp"
#include "Function/General/ComputeShader.hpp"

#include "Component/Animator.hpp"

namespace aEngine {

void DeformSkinnedMesh(Render::Mesh *mesh, Animator &animator);

void BuildSkeletonHierarchy(Entity *root,
                            std::map<std::string, Entity *> &hierarchyMap);

};