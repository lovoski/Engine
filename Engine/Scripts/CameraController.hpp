/**
 * Update active camera in the scene with user input.
 */
#pragma once

#include "Base/Scriptable.hpp"
#include "Component/Camera.hpp"
#include "Scene.hpp"

#include "Function/Math/Math.hpp"
#include "Function/Render/VisUtils.hpp"

namespace aEngine {

struct EditorCameraController : public Scriptable {
  bool mouseFirstMove = true;
  glm::vec2 mouseLastPos;

  glm::vec3 cameraPivot = glm::vec3(0.0f);

  // Some parameter related to camera control
  float initialOffset = 0.01f;
  float initialFactor = 0.6f;
  float speedPow = 1.5f;
  float maxSpeed = 8e2f;

  void Update(float dt) override {
    EntityID camera;
    if (GWORLD.GetActiveCamera(camera)) {
      auto cameraObject = GWORLD.EntityFromID(camera);
      auto camPos = cameraObject->Position();
      auto sceneContext = GWORLD.Context;
      float movementSpeed =
          initialOffset +
          initialFactor * dt *
              std::min(std::pow(glm::length(camPos - cameraPivot), speedPow),
                       maxSpeed);
      bool inSceneWindow =
          GWORLD.InSceneWindow(sceneContext.currentMousePosition.x,
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
          GWORLD.LoopCursorInSceneWindow()) {
        // loop the camera if the camera is locked
        if (mouseFirstMove) {
          mouseLastPos = mouseCurrentPos;
          mouseFirstMove = false;
        }
        glm::vec2 mouseOffset = 0.1f * (mouseCurrentPos - mouseLastPos);
        if (sceneContext.engine->GetKey(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
          // move the view position
          glm::vec3 positionDelta =
              -movementSpeed * mouseOffset.x * cameraObject->LocalLeft +
              movementSpeed * mouseOffset.y * cameraObject->LocalUp;
          cameraPivot += positionDelta;
          cameraObject->SetGlobalPosition(cameraObject->Position() +
                                          positionDelta);
        } else {
          // repose the camera
          glm::vec3 posVector = cameraObject->Position();
          glm::quat rotY = glm::angleAxis(glm::radians(-mouseOffset.x),
                                          glm::vec3(0.0f, 1.0f, 0.0f));
          glm::quat rotX = glm::angleAxis(glm::radians(-mouseOffset.y),
                                          cameraObject->LocalLeft);
          glm::vec3 newPos =
              rotY * rotX * (posVector - cameraPivot) + cameraPivot;
          cameraObject->SetGlobalPosition(newPos);
        }
        mouseLastPos = mouseCurrentPos;
      } else
        mouseFirstMove = true;
      glm::vec3 lastLeft = cameraObject->LocalLeft;
      glm::vec3 forward = normalize(cameraObject->Position() - cameraPivot);
      glm::vec3 up = Entity::WorldUp;
      glm::vec3 left = normalize(cross(up, forward));
      // flip left if non-consistent
      if (glm::dot(lastLeft, left) < 0.0f)
        left *= -1;
      up = normalize(cross(forward, left));
      glm::mat3 rot(left, up, forward);
      cameraObject->SetGlobalRotation(glm::quat_cast(rot));
    }
  }

  // void DrawToScene() override {
  //   EntityID camera;
  //   if (GWORLD.GetActiveCamera(camera)) {
  //     auto cameraComp = GWORLD.GetComponent<Camera>(camera);
  //     VisUtils::DrawSquare(cameraPivot, 1.0f, cameraComp->VP,
  //                          GWORLD.Context.sceneWindowSize,
  //                          glm::vec3(1.0f, 0.0f, 0.0f));
  //   }
  // }

  void DrawInspectorGUI() override {
    DrawInspectorGUIDefault();
    ImGui::SliderFloat("offset", &initialOffset, 0.0f, 1.0f);
    ImGui::SliderFloat("sensitiviy", &initialFactor, 0.0f, 2.0f);
    ImGui::SliderFloat("pow", &speedPow, 1.0f, 3.0f);
    ImGui::SliderFloat("upper bound", &maxSpeed, 100, 1e3);
    float pivot[3] = {cameraPivot.x, cameraPivot.y, cameraPivot.z};
    if (ImGui::DragFloat3("camera pivot", pivot, 0.01f,
                          -std::numeric_limits<float>::max() / 3,
                          std::numeric_limits<float>::max() / 3)) {
      cameraPivot.x = pivot[0];
      cameraPivot.y = pivot[1];
      cameraPivot.z = pivot[2];
    }
  }
};

}; // namespace aEngine
