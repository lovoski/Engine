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
      if (cameraComp.moveScheme == Camera::MOVEMENT_STYLE::FPS_CAMERA) {
        // update the camera with predefined fps camera way
        float detltaTime = Timer.DeltaTime();
        float movementDistance = cameraComp.movementSpeed * detltaTime;
        float mouseSensitivity = cameraComp.mouseSensitivity;

        vec3 cameraForward = transform.Rotation * Transform::WorldForward;
        vec3 cameraUp = transform.Rotation * Transform::WorldUp;
        vec3 cameraRight = transform.Rotation * Transform::WorldRight;

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

        Console.AddLog("camera pos x=%f, y=%f, z=%f\n", transform.Position.x, transform.Position.y, transform.Position.z);

        // change rotation of camera
        if (Event.GetMouseButton(GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS &&
            EditorContext.InSceneWindow(Event.MouseCurrentPosition.x,
                                        Event.MouseCurrentPosition.y)) {
          float offsetLength =
              Event.MousePositionOffset.x * Event.MousePositionOffset.x +
              Event.MousePositionOffset.y * Event.MousePositionOffset.y;
          if (offsetLength > 0.5f) {
            vec2 mouseOffset = mouseSensitivity * Event.MousePositionOffset;
            mouseOffset.y *= -1;
            float xAddAngle = glm::radians(mouseOffset.x);
            float yAddAngle = glm::radians(mouseOffset.y);
            quat xAddRot(cos(xAddAngle / 2), sin(xAddAngle / 2) * cameraUp);
            quat yAddRot(cos(yAddAngle / 2), sin(yAddAngle / 2) * cameraRight);
            transform.Rotation = xAddRot * yAddRot * transform.Rotation;
          }
        }
      }
    }
  }

private:
  ECS::EntityID activeCamera;
};