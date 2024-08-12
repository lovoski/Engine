#include "Entity.hpp"
#include "Scene.hpp"

namespace aEngine {

void Entity::Destroy() { scene->DestroyEntity(ID); }

void Entity::AssignChild(Entity *c) {
  if (c == nullptr)
    throw std::runtime_error("can't set null pointer as child");
  // make sure c is not a ancestor of current entity
  if (parent != nullptr) {
    auto current = parent;
    while (current != nullptr) {
      if (current == c) {
        printf("can't set directly revert ancestor child relation\n");
        return;
      }
      current = current->parent;
    }
  }
  if (std::find(children.begin(), children.end(), c) != children.end()) {
    printf("entity %d is already the child of entity %d\n", (unsigned int)c->ID,
           (unsigned int)ID);
    return;
  }
  if (c->parent != nullptr) {
    // remove c from its parent's list
    auto it =
        std::find(c->parent->children.begin(), c->parent->children.end(), c);
    if (it == c->parent->children.end())
      throw std::runtime_error("entity is not a child of its parent");
    c->parent->children.erase(it);
  }
  children.push_back(c);
  c->parent = this;
  // update the local properties with global properties
  c->SetGlobalPosition(c->Position());
  c->SetGlobalRotation(c->Rotation());
  c->SetGlobalScale(c->Scale());
}

void Entity::GetParentLocalAxis(glm::vec3 &pLocalForward, glm::vec3 &pLocalLeft,
                                glm::vec3 &pLocalUp) {
  if (parent == nullptr) {
    pLocalForward = WorldForward;
    pLocalLeft = WorldLeft;
    pLocalUp = WorldUp;
  } else {
    parent->UpdateLocalAxis();
    pLocalForward = parent->LocalForward;
    pLocalLeft = parent->LocalLeft;
    pLocalUp = parent->LocalUp;
  }
}

void Entity::Serialize(Json &json) {
  json["p"] = m_position;
  json["r"] = m_eulerAngles;
  json["s"] = m_scale;
  json["parent"] = parent == nullptr ? "none" : std::to_string((int)parent->ID);
  json["name"] = name;
}

}; // namespace aEngine