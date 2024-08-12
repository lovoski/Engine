/**
 * Update active camera in the scene with user input.
 */
#pragma once

#include "Scene.hpp"
#include "Base/Scriptable.hpp"
#include "Component/Camera.hpp"

#include "Utils/Render/VisUtils.hpp"

namespace aEngine {

struct CameraController : public Scriptable {
  bool mouseFirstMove = true;
  glm::vec2 mouseLastPos;

  void Update() override {
    EntityID camera;
    auto scene = entity->scene;
    if (scene->GetActiveCamera(camera)) {
      auto cameraObject = scene->EntityFromID(camera);
      auto sceneContext = scene->Context;
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
                                                scrollOffset.y * 0.1f);
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
              0.1f * mouseOffset.x * cameraObject->LocalLeft +
              0.1f * mouseOffset.y * cameraObject->LocalUp);
        } else {
          // move the view direction
          cameraObject->SetGlobalRotation(cameraObject->EulerAngles() -
                                          glm::vec3(glm::radians(mouseOffset.y),
                                                    glm::radians(mouseOffset.x),
                                                    0.0f));
        }
        mouseLastPos = mouseCurrentPos;
      } else
        mouseFirstMove = true;
    }
  }
};

}; // namespace aEngine
