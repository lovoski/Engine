#pragma once

#include "Base/Types.hpp"
#include "Global.hpp"
#include "Scene.hpp"

namespace aEngine {

class Entity {
public:
  friend class Scene;

  Entity(EntityID id, Scene *s) : ID(id), scene(s) {
    m_scale = glm::vec3(1.0f);
    m_position = glm::vec3(0.0f);
    m_rotation = glm::quat(1.0f, glm::vec3(0.0f));
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
  const glm::vec3 EulerAngles() { return glm::eulerAngles(m_rotation); }
  // global rotation
  const glm::quat Rotation() { return m_rotation; }
  // position under world axis
  const glm::vec3 Position() { return m_position; }

  const glm::vec3 LocalPosition() { return localPosition; }
  const glm::quat LocalRotation() { return localRotation; }
  const glm::vec3 LocalScale() { return localScale; }

  void SetLocalPosition(glm::vec3 p) {
    localPosition = p;
    transformDirty = true;
  }
  void SetLocalRotation(glm::quat q) {
    localRotation = q;
    transformDirty = true;
  }
  void SetLocalScale(glm::vec3 s) {
    localScale = s;
    transformDirty = true;
  }

  static glm::vec3 WorldUp, WorldLeft, WorldForward;

  // local axis are updated at the start of each loop
  glm::vec3 LocalUp, LocalLeft, LocalForward;

  bool Enabled = true;

  // set global position
  void SetGlobalPosition(glm::vec3 p);
  // set global rotation, mark quatDirty to true
  void SetGlobalRotation(glm::quat q);
  // set global scale
  void SetGlobalScale(glm::vec3 s);

  void UpdateLocalAxis();

  // global position to the local position relative to its parent
  const glm::vec3 GlobalToLocal(glm::vec3 globalPos);
  // localposition relative to its parent to global position
  const glm::vec3 LocalToGlobal(glm::vec3 localPos);

  const glm::vec3 GetParentScale();

  const glm::vec3 GetParentPosition();

  // (self.orien = parent.orien * self.localRot)
  // or (self.globalRot = parent.globalRot * self.localRot)
  const glm::quat GetParentOrientation();

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

  glm::mat4 GetModelMatrix();

  EntityID ID;
  Scene *scene;
  std::string name = "New Entity ";
  Entity *parent = nullptr;
  std::vector<Entity *> children;

protected:
  bool transformDirty = true;

  // scale relative to its parent's axis
  glm::vec3 localScale = glm::vec3(1.0f);
  // rotation relative to its parent's axis
  // euler angles are hard to convert be local, use quaternion only
  glm::quat localRotation = glm::quat(1.0f, glm::vec3(0.0f));
  // position relative to its parent's axis
  glm::vec3 localPosition = glm::vec3(0.0f);

  glm::vec3 m_position;
  glm::vec3 m_scale;
  glm::quat m_rotation;
};

}; // namespace aEngine