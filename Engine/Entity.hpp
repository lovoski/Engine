#pragma once

#include "Global.hpp"
#include "Base/Types.hpp"

namespace aEngine {

class Entity {
public:
  friend class Scene;

  Entity(EntityID id, Scene *scene) : ID(id), scene(scene) {
    m_scale = glm::vec3(1.0f);
    m_position = glm::vec3(0.0f);
    m_eulerAngles = glm::vec3(0.0f);
  }
  ~Entity() {
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

  // global scale
  const glm::vec3 Scale() { return m_scale; };
  // global rotation (pitch, yaw, roll) = (x, y, z)
  // in radians
  const glm::vec3 EulerAngles() { return m_eulerAngles; }
  // euler angles in degree
  const glm::vec3 EulerAnglesDegree() { return glm::degrees(m_eulerAngles); }
  // global rotation
  const glm::quat Rotation() { return glm::quat(m_eulerAngles); }
  // position under world axis
  const glm::vec3 Position() { return m_position; }

  // scale relative to its parent's axis
  glm::vec3 localScale = glm::vec3(1.0f);
  // rotation relative to its parent's axis
  glm::quat localRotation = glm::quat(1.0f, glm::vec3(0.0f));
  // position relative to its parent's axis
  glm::vec3 localPosition = glm::vec3(0.0f);

  static glm::vec3 WorldUp, WorldLeft, WorldForward;

  // local axis are updated at the start of each loop
  glm::vec3 LocalUp, LocalLeft, LocalForward;

  bool Enabled = true;

  // set global position
  void SetGlobalPosition(glm::vec3 p) {
    // change global position, modify local position to satisfy the global
    // position
    m_position = p;
    // the local axis will be updated in GlobalToLocal function call
    localPosition = GlobalToLocal(p);
  }
  // set global rotation (pitch, yaw, roll) = (x, y, z)
  // in degrees
  void SetGlobalRotationDegree(glm::vec3 a) {
    a = glm::radians(a);
    SetGlobalRotation(a);
  }
  // set global rotation (pitch, yaw, roll) = (x, y, z)
  // in radians
  void SetGlobalRotation(glm::vec3 a) {
    glm::quat parentOrien = GetParentOrientation();
    localRotation = glm::inverse(parentOrien) * glm::quat(a);
    m_eulerAngles = a;
    UpdateLocalAxis();
  }
  // set global rotation
  void SetGlobalRotation(glm::quat q) {
    glm::quat parentOrien = GetParentOrientation();
    localRotation = glm::inverse(parentOrien) * q;
    m_eulerAngles = glm::eulerAngles(q);
    UpdateLocalAxis();
  }
  // set global scale
  void SetGlobalScale(glm::vec3 s) {
    localScale = s / GetParentScale();
    m_scale = s;
  }

  void UpdateLocalAxis() {
    glm::quat q = GetParentOrientation() * localRotation;
    LocalForward = q * WorldForward;
    LocalLeft = q * WorldLeft;
    LocalUp = q * WorldUp;
  }

  // global position to the local position relative to its parent
  const glm::vec3 GlobalToLocal(glm::vec3 globalPos) {
    glm::vec3 pLocalForward, pLocalLeft, pLocalUp;
    GetParentLocalAxis(pLocalForward, pLocalLeft, pLocalUp);
    // TODO: always remembers how to intiailize a matrix
    glm::mat3 M_p(pLocalLeft, pLocalUp, pLocalForward);
    glm::mat3 M(WorldLeft, WorldUp, WorldForward);
    return glm::inverse(M_p) * M * (globalPos - GetParentPosition());
  }
  // localposition relative to its parent to global position
  const glm::vec3 LocalToGlobal(glm::vec3 localPos) {
    glm::vec3 pLocalForward, pLocalLeft, pLocalUp;
    GetParentLocalAxis(pLocalForward, pLocalLeft, pLocalUp);
    glm::mat3 M_p(pLocalLeft, pLocalUp, pLocalForward);
    glm::mat3 M(WorldLeft, WorldUp, WorldForward);
    return (glm::inverse(M) * M_p * localPos) + GetParentPosition();
  }

  const glm::vec3 GetParentScale() {
    if (parent == nullptr)
      return glm::vec3(1.0f);
    else
      return parent->m_scale;
  }

  const glm::vec3 GetParentPosition() {
    if (parent == nullptr)
      return glm::vec3(0.0f);
    else
      return parent->m_position;
  }

  // (self.orien = parent.orien * self.localRot)
  // or (self.globalRot = parent.globalRot * self.localRot)
  const glm::quat GetParentOrientation() {
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

  void GetParentLocalAxis(glm::vec3 &pLocalForward, glm::vec3 &pLocalLeft,
                          glm::vec3 &pLocalUp);

  void Serialize(Json &json);

  template <typename T, typename... Args> void AddComponent(Args &&...args) {
    scene->AddComponent<T>(ID, std::forward<Args>(args)...);
  }

  template <typename T> void RemoveComponent() {
    scene->RemoveComponent<T>(ID);
  }

  template <typename T> const bool HasComponent() {
    return scene->HasComponent<T>(ID);
  }

  template <typename T> T &GetComponent() { return scene->GetComponent<T>(ID); }

  void Destroy();

  void AssignChild(Entity *c);

  glm::mat4 GetModelMatrix() {
    return glm::translate(glm::mat4(1.0f), m_position) *
           glm::mat4_cast(Rotation()) * glm::scale(glm::mat4(1.0f), m_scale);
  }

  EntityID ID;
  Scene *scene;
  std::string name = "New Entity ";
  Entity *parent = nullptr;
  std::vector<Entity *> children;

protected:
  glm::vec3 m_position;
  glm::vec3 m_scale;
  glm::vec3 m_eulerAngles;
};

}; // namespace aEngine