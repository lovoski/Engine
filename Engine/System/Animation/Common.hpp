#pragma once

#include "Global.hpp"
#include "Scene.hpp"
#include "Entity.hpp"

namespace aEngine {

void BuildSkeletonHierarchy(Entity *root,
                            std::map<std::string, Entity *> &hierarchyMap);

};