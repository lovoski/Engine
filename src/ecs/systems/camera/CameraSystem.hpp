#pragma once

#include "basics.hpp"
#include "ecs/components/Camera.hpp"
#include "ecs/ecs.hpp"

// controls the movement of camera
class CameraSystem : public ECS::BaseSystem {
public:
  CameraSystem() { AddComponentSignature<Camera>(); }
  ~CameraSystem() {}

  void Update() {
    if (EditorContext.GetActiveCamera(activeCamera)) {
      // update the position of this active camera
      auto cameraObject = ECS::EManager.EntityFromID(activeCamera);
      auto &cameraComp = ECS::EManager.GetComponent<Camera>(activeCamera);
      float detltaTime = Timer.DeltaTime();
      float movementDistance = cameraComp.movementSpeed * detltaTime;
      float mouseSensitivity = cameraComp.mouseSensitivity;
      if (cameraComp.moveScheme == Camera::MOVEMENT_STYLE::FPS_CAMERA) {
        // update the camera with predefined fps camera way
        bool inSceneWindow = EditorContext.InSceneWindow(
            Event.MouseCurrentPosition.x, Event.MouseCurrentPosition.y);
        if (inSceneWindow) {
          // check action queue for mouse scroll event
          for (auto action : Event.actions) {
            if (action.Type == Action::ActionType::MOUSE_SCROLL) {
              vec2 scrollOffset = (*(vec2 *)action.Data);
              cameraComp.fovY += scrollOffset.y;
            }
          }
          // change position of camera
          if (Event.GetKey(GLFW_KEY_W) == GLFW_PRESS)
            cameraObject->SetGlobalPosition(cameraObject->Position() -
                                            movementDistance *
                                                cameraObject->LocalForward);
          if (Event.GetKey(GLFW_KEY_A) == GLFW_PRESS)
            cameraObject->SetGlobalPosition(cameraObject->Position() -
                                            movementDistance *
                                                cameraObject->LocalLeft);
          if (Event.GetKey(GLFW_KEY_S) == GLFW_PRESS)
            cameraObject->SetGlobalPosition(cameraObject->Position() +
                                            movementDistance *
                                                cameraObject->LocalForward);
          if (Event.GetKey(GLFW_KEY_D) == GLFW_PRESS)
            cameraObject->SetGlobalPosition(cameraObject->Position() +
                                            movementDistance *
                                                cameraObject->LocalLeft);
          if (Event.GetKey(GLFW_KEY_SPACE) == GLFW_PRESS)
            cameraObject->SetGlobalPosition(cameraObject->Position() +
                                            movementDistance *
                                                cameraObject->LocalUp);
          if (Event.GetKey(GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
            cameraObject->SetGlobalPosition(cameraObject->Position() -
                                            movementDistance *
                                                cameraObject->LocalUp);
        }
        // change rotation of camera
        vec2 mouseCurrentPos = Event.MouseCurrentPosition;
        // the shift only happens if cousor in the range of scene window
        if (Event.GetKey(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS &&
            EditorContext.LoopCursorInSceneWindow()) {
          // loop the camera if the camera is locked
          if (mouseFirstMove) {
            mouseLastPos = mouseCurrentPos;
            mouseFirstMove = false;
          }
          vec2 mouseOffset =
              mouseSensitivity * (mouseCurrentPos - mouseLastPos);
          cameraObject->SetGlobalRotation(cameraObject->EulerAngles() -
                                          vec3(glm::radians(mouseOffset.y),
                                               glm::radians(mouseOffset.x),
                                               0.0f));
          mouseLastPos = mouseCurrentPos;
        } else
          mouseFirstMove = true;
      } else if (cameraComp.moveScheme ==
                 Camera::MOVEMENT_STYLE::MOUSE_KEY_CAMERA) {
        bool inSceneWindow = EditorContext.InSceneWindow(
            Event.MouseCurrentPosition.x, Event.MouseCurrentPosition.y);
        if (inSceneWindow) {
          // check action queue for mouse scroll event
          for (auto action : Event.actions) {
            if (action.Type == Action::ActionType::MOUSE_SCROLL) {
              vec2 scrollOffset = (*(vec2 *)action.Data);
              cameraObject->SetGlobalPosition(cameraObject->Position() - cameraObject->LocalForward * scrollOffset.y * 0.1f);
            }
          }
        }
        vec2 mouseCurrentPos = Event.MouseCurrentPosition;
        if (Event.GetMouseButton(GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS &&
            EditorContext.LoopCursorInSceneWindow()) {
          // loop the camera if the camera is locked
          if (mouseFirstMove) {
            mouseLastPos = mouseCurrentPos;
            mouseFirstMove = false;
          }
          vec2 mouseOffset =
              mouseSensitivity * (mouseCurrentPos - mouseLastPos);
          if (Event.GetKey(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
            // move the view position
            cameraObject->SetGlobalPosition(
                cameraObject->Position() -
                0.1f * mouseOffset.x * cameraObject->LocalLeft +
                0.1f * mouseOffset.y * cameraObject->LocalUp);
          } else {
            // move the view direction
            cameraObject->SetGlobalRotation(cameraObject->EulerAngles() -
                                            vec3(glm::radians(mouseOffset.y),
                                                 glm::radians(mouseOffset.x),
                                                 0.0f));
          }
          mouseLastPos = mouseCurrentPos;
        } else
          mouseFirstMove = true;
      }
    }
  }

private:
  ECS::EntityID activeCamera;

  bool mouseFirstMove = true;
  vec2 mouseLastPos = vec2(0.0f);
};