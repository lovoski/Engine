/**
 * Update active camera in the scene with user input.
 */
#pragma once

#include "Base/Scriptable.hpp"
#include "Component/Camera.hpp"
#include "Scene.hpp"

#include "Utils/Render/VisUtils.hpp"

namespace aEngine {

struct CameraController : public Scriptable {
  bool mouseFirstMove = true;
  glm::vec2 mouseLastPos;

  glm::vec3 cameraEuler;

  void Start() override {
    EntityID camera;
    if (GWORLD.GetActiveCamera(camera)) {
      auto cameraObject = GWORLD.EntityFromID(camera);
      cameraEuler = cameraObject->EulerAngles();
    }
  }

  void Update(float dt) override {
    EntityID camera;
    auto scene = entity->scene;
    if (scene->GetActiveCamera(camera)) {
      auto cameraObject = scene->EntityFromID(camera);
      auto camPos = cameraObject->Position();
      auto sceneContext = scene->Context;
      // float movementSpeed =
      //     0.01f + 0.01f * abs(glm::dot(glm::vec3(camPos.x, 0.0f, camPos.z),
      //                              cameraObject->LocalForward));
      float movementSpeed = 0.01f + dt * std::min(std::pow(glm::length(camPos), 1.6f), 4e2f);
      bool inSceneWindow =
          scene->InSceneWindow(sceneContext.currentMousePosition.x,
                               sceneContext.currentMousePosition.y);
      if (inSceneWindow) {
        // check action queue for mouse scroll event
        for (auto action : sceneContext.engine->ActionQueue) {
          if (action.type == ACTION_TYPE::MOUSE_SCROLL) {
            glm::vec2 scrollOffset = (*(glm::vec2 *)action.payload);
            cameraObject->SetGlobalPosition(cameraObject->Position() -
                                            cameraObject->LocalForward *
                                                scrollOffset.y * movementSpeed);
          }
        }
      }
      glm::vec2 mouseCurrentPos = sceneContext.currentMousePosition;
      if (sceneContext.engine->GetMouseButton(GLFW_MOUSE_BUTTON_MIDDLE) ==
              GLFW_PRESS &&
          scene->LoopCursorInSceneWindow()) {
        // loop the camera if the camera is locked
        if (mouseFirstMove) {
          mouseLastPos = mouseCurrentPos;
          mouseFirstMove = false;
        }
        glm::vec2 mouseOffset = 0.1f * (mouseCurrentPos - mouseLastPos);
        if (sceneContext.engine->GetKey(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
          // move the view position
          cameraObject->SetGlobalPosition(
              cameraObject->Position() -
              movementSpeed * mouseOffset.x * cameraObject->LocalLeft +
              movementSpeed * mouseOffset.y * cameraObject->LocalUp);
        } else {
          // move the view direction
          cameraEuler -= glm::radians(glm::vec3(mouseOffset.y, mouseOffset.x, 0.0f));
          cameraObject->SetGlobalRotation(glm::quat(cameraEuler));
        }
        mouseLastPos = mouseCurrentPos;
      } else
        mouseFirstMove = true;
    }
  }
};

}; // namespace aEngine
