#include "Component/Camera.hpp"

namespace aEngine {

void Camera::GetCameraViewPerpProjMatrix(glm::mat4 &view, glm::mat4 &proj) {
  auto size = GWORLD.Context.sceneWindowSize;
  if (GWORLD.EntityValid(entityID)) {
    auto entity = GWORLD.EntityFromID(entityID);
    proj = glm::perspective(glm::radians(fovY), size.x / size.y, zNear, zFar);
    view =
        glm::lookAt(entity->Position(),
                    entity->Position() - entity->LocalForward, entity->LocalUp);
  } else
    LOG_F(ERROR, "Entity of camera component not valid");
}

void Camera::DrawInspectorGUI() {
  ImGui::DragFloat("FovY", &fovY, 1.0f, 0.0f, 150.0f);
  ImGui::DragFloat("zNear", &zNear, 0.001f, 0.0000001f, 10.0f);
  ImGui::DragFloat("zFar", &zFar, 0.1f, 20.0f, 2000.0f);
}

}; // namespace aEngine

REGISTER_COMPONENT(aEngine, Camera);