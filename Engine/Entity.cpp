#include "Entity.hpp"
#include "Scene.hpp"

namespace aEngine {

glm::vec3 Entity::WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 Entity::WorldLeft = glm::vec3(1.0f, 0.0f, 0.0f);
glm::vec3 Entity::WorldForward = glm::vec3(0.0f, 0.0f, 1.0f);

Entity::~Entity() { LOG_F(1, "deconstruct entity %s", name.c_str()); }

void Entity::Destroy() {
  LOG_F(1, "destroy parent child relation for %s", name.c_str());
  if (parent != nullptr) {
    // remove this child from its parent's child list
    if (parent->children.size() != 0) {
      auto it =
          std::find(parent->children.begin(), parent->children.end(), this);
      if (it != parent->children.end())
        parent->children.erase(it);
    }
    parent = nullptr;
  }
  children.clear();
}

void Entity::AssignChild(Entity *c) {
  if (c == nullptr)
    throw std::runtime_error("can't set null pointer as child");
  // make sure c is not a ancestor of current entity
  if (parent != nullptr) {
    auto current = parent;
    while (current != nullptr) {
      if (current == c) {
        LOG_F(WARNING, "can't set directly revert ancestor child relation");
        return;
      }
      current = current->parent;
    }
  }
  if (std::find(children.begin(), children.end(), c) != children.end()) {
    LOG_F(WARNING, "entity %d is already the child of entity %d",
          (unsigned int)c->ID, (unsigned int)ID);
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

void Entity::SetGlobalPosition(glm::vec3 p) {
  // change global position, modify local position to satisfy the global
  // position
  m_position = p;
  // the local axis will be updated in GlobalToLocal function call
  localPosition = GlobalToLocal(p);
  transformDirty = true;
}

void Entity::SetGlobalRotation(glm::quat q) {
  m_rotation = q;
  // update local rotation
  glm::quat parentOrien = GetParentOrientation();
  localRotation = glm::inverse(parentOrien) * q;
  UpdateLocalAxis();
  transformDirty = true;
}

void Entity::SetGlobalScale(glm::vec3 s) {
  localScale = s / GetParentScale();
  m_scale = s;
  transformDirty = true;
}

void Entity::UpdateLocalAxis() {
  glm::quat q = GetParentOrientation() * localRotation;
  LocalForward = q * WorldForward;
  LocalLeft = q * WorldLeft;
  LocalUp = q * WorldUp;
}

const glm::vec3 Entity::GlobalToLocal(glm::vec3 globalPos) {
  glm::vec3 pLocalForward, pLocalLeft, pLocalUp;
  GetParentLocalAxis(pLocalForward, pLocalLeft, pLocalUp);
  // TODO: always remembers how to intiailize a matrix
  glm::mat3 M_p(pLocalLeft, pLocalUp, pLocalForward);
  glm::mat3 M(WorldLeft, WorldUp, WorldForward);
  return glm::inverse(M_p) * M * (globalPos - GetParentPosition());
}

const glm::vec3 Entity::LocalToGlobal(glm::vec3 localPos) {
  glm::vec3 pLocalForward, pLocalLeft, pLocalUp;
  GetParentLocalAxis(pLocalForward, pLocalLeft, pLocalUp);
  glm::mat3 M_p(pLocalLeft, pLocalUp, pLocalForward);
  glm::mat3 M(WorldLeft, WorldUp, WorldForward);
  return (glm::inverse(M) * M_p * localPos) + GetParentPosition();
}

const glm::vec3 Entity::GetParentScale() {
  if (parent == nullptr)
    return glm::vec3(1.0f);
  else
    return parent->m_scale;
}

const glm::vec3 Entity::GetParentPosition() {
  if (parent == nullptr)
    return glm::vec3(0.0f);
  else
    return parent->m_position;
}

const glm::quat Entity::GetParentOrientation() {
  Entity *current = parent;
  glm::quat q(1.0f, glm::vec3(0.0f)); // root.parent.orien
  std::stack<glm::quat> s;
  while (current != nullptr) {
    s.push(current->localRotation); // cur.localRot
    current = current->parent;
  }
  while (!s.empty()) {
    q = q * s.top();
    s.pop();
  }
  return q;
}

glm::mat4 Entity::GetModelMatrix() {
  return glm::translate(glm::mat4(1.0f), m_position) *
         glm::mat4_cast(Rotation()) * glm::scale(glm::mat4(1.0f), m_scale);
}

void Entity::Serialize(Json &json) {
  json["p"] = m_position;
  json["r"] = m_rotation;
  json["s"] = m_scale;
  json["parent"] = parent == nullptr ? "none" : std::to_string((int)parent->ID);
  json["name"] = name;
}

}; // namespace aEngine