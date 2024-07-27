#pragma once

#include "basics.hpp"
#include "ecs/components/Camera.hpp"
#include "ecs/ecs.hpp"

class CameraSystem : public ECS::BaseSystem {
public:
  CameraSystem() { AddComponentSignature<Camera>(); }
  ~CameraSystem() {}

  void Update() {
    if (EditorContext.GetActiveCamera(activeCamera)) {
      // update the position of this active camera
      auto &cameraComp = ECS::Manager.GetComponent<Camera>(activeCamera);
      auto &transform = ECS::Manager.GetComponent<Transform>(activeCamera);
      float detltaTime = Timer.DeltaTime();
      float movementDistance = cameraComp.movementSpeed * detltaTime;
      float mouseSensitivity = cameraComp.mouseSensitivity;

      vec3 cameraForward = transform.Rotation * Transform::WorldForward;
      vec3 cameraUp = transform.Rotation * Transform::WorldUp;
      vec3 cameraRight = transform.Rotation * Transform::WorldRight;

      if (cameraComp.moveScheme == Camera::MOVEMENT_STYLE::FPS_CAMERA) {
        // update the camera with predefined fps camera way
        bool inSceneWindow = EditorContext.InSceneWindow(
            Event.MouseCurrentPosition.x, Event.MouseCurrentPosition.y);
        if (inSceneWindow) {
          // check action queue for mouse scroll event
          for (auto action : Event.actions) {
            if (action.Type == Action::ActionType::MOUSE_SCROLL) {
              vec2 scrollOffset = (*(vec2 *)action.Data);
              cameraComp.fovY += glm::radians(scrollOffset.y);
            }
          }
          // change position of camera
          if (Event.GetKey(GLFW_KEY_W) == GLFW_PRESS)
            transform.Position += movementDistance * cameraForward;
          if (Event.GetKey(GLFW_KEY_A) == GLFW_PRESS)
            transform.Position -= movementDistance * cameraRight;
          if (Event.GetKey(GLFW_KEY_D) == GLFW_PRESS)
            transform.Position += movementDistance * cameraRight;
          if (Event.GetKey(GLFW_KEY_S) == GLFW_PRESS)
            transform.Position -= movementDistance * cameraForward;
          if (Event.GetKey(GLFW_KEY_SPACE) == GLFW_PRESS)
            transform.Position += movementDistance * cameraUp;
          if (Event.GetKey(GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            transform.Position -= movementDistance * cameraUp;
        }
        // change rotation of camera
        if (Event.GetKey(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS &&
            EditorContext.LoopCursorInSceneWindow()) {
          vec2 mouseCurrentPos = Event.MouseCurrentPosition;
          if (mouseFirstMove) {
            mouseLastPos = mouseCurrentPos;
            mouseFirstMove = false;
          }
          vec2 mouseOffset =
              mouseSensitivity * (mouseCurrentPos - mouseLastPos);
          float xAddAngle = glm::radians(mouseOffset.x);
          float yAddAngle = glm::radians(mouseOffset.y);
          quat xAddRot(cos(xAddAngle / 2), sin(xAddAngle / 2) * cameraUp);
          quat yAddRot(cos(yAddAngle / 2), sin(yAddAngle / 2) * cameraRight);
          transform.Rotation = xAddRot * yAddRot * transform.Rotation;
          mouseLastPos = mouseCurrentPos;
        } else {
          mouseFirstMove = true;
        }
      }
    }
  }

private:
  ECS::EntityID activeCamera;

  bool mouseFirstMove = true;
  vec2 mouseLastPos = vec2(0.0f);
};