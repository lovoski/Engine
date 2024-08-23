#include "System/Animation/Common.hpp"

namespace aEngine {

void BuildSkeletonHierarchy(Entity *root,
                            std::map<std::string, Entity *> &hierarchyMap) {
  std::queue<EntityID> q;
  q.push(root->ID);
  while (!q.empty()) {
    EntityID cur = q.front();
    Entity *curEnt = GWORLD.EntityFromID(cur);
    hierarchyMap.insert(std::make_pair(curEnt->name, curEnt));
    q.pop();
    for (auto c : curEnt->children)
      q.push(c->ID);
  }
}

};