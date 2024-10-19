#include "Scripts/Animation/IK/TwoBoneIK.hpp"
#include "Function/Animation/Motion.hpp"
#include "Function/GUI/Helpers.hpp"
#include "Function/Math/Math.hpp"
#include "Function/Render/VisUtils.hpp"

namespace aEngine {

void twoboneikDragableTarget(Entity **joint, std::vector<char> &namebuffer,
                             std::string label);

void TwoBoneIK::LateUpdate(float dt) {
  if (joint0 && joint1 && joint2 && target && pole) {
    auto p0 = joint0->Position(), p1 = joint1->Position(),
         p2 = joint2->Position(), pt = target->Position();
    auto dir01 = p1 - p0, dir12 = p2 - p1;
    float l0 = glm::length(dir01), l1 = glm::length(dir12);
    float dist = glm::length(pt - p0);
    auto dir = glm::normalize(pt - p0);
    auto normal =
        glm::normalize(glm::cross(glm::normalize(pole->Position() - p0),
                                  glm::normalize(p2 - pole->Position())));
    if (glm::abs(dist - l0 - l1) < 1e-3f) {
      p1 = p0 + l0 * dir;
      p2 = p0 + (l0 + l1) * dir;
    } else {
      auto h = glm::normalize(glm::cross(dir, normal));
      float cosalpha = (l0 * l0 + dist * dist - l1 * l1) / (2.0f * l0 * dist);
      p1 = glm::sqrt(glm::abs(1.0f - cosalpha * cosalpha)) * l0 * h +
           cosalpha * l0 * dir + p0;
      p2 = pt;
    }
    joint0->SetGlobalRotation(Math::LookAtRotation(
        glm::cross(glm::normalize(p1 - p0), normal), glm::normalize(p1 - p0)));
    joint1->SetGlobalRotation(Math::LookAtRotation(
        glm::cross(glm::normalize(p2 - p1), normal), glm::normalize(p2 - p1)));
    joint2->SetGlobalRotation(target->Rotation());
  }
}

void TwoBoneIK::DrawToScene() {
  EntityID camera;
  if (GWORLD.GetActiveCamera(camera)) {
    auto vp = GWORLD.GetComponent<Camera>(camera)->VP;
    if (target)
      VisUtils::DrawWireSphere(target->Position(), vp, 10.0f, VisUtils::Red);
    if (pole)
      VisUtils::DrawWireSphere(pole->Position(), vp, 10.0f, VisUtils::Yellow);
  }
}

void TwoBoneIK::DrawInspectorGUI() {
  ImGui::MenuItem("Objects", nullptr, nullptr, false);
  twoboneikDragableTarget(&joint0, j0NameBuffer, "Joint 0");
  twoboneikDragableTarget(&joint1, j1NameBuffer, "Joint 1");
  twoboneikDragableTarget(&joint2, j2NameBuffer, "Joint 2");
  twoboneikDragableTarget(&pole, poleNameBuffer, "Pole Object");
  twoboneikDragableTarget(&target, targetNameBuffer, "Target Object");

  ImGui::Separator();
  if (ImGui::Button("Create Target and Pole", {-1, 30})) {
    if (joint0 && joint1 && joint2) {
      auto c1 = GWORLD.AddNewEntity();
      c1->SetGlobalPosition(joint2->Position());
      target = c1.get();
      auto c2 = GWORLD.AddNewEntity();
      auto dir = joint2->Position() - joint0->Position();
      auto h = glm::cross(Entity::WorldUp, glm::normalize(dir));
      auto dist = glm::length(dir);
      c2->SetGlobalPosition(0.5f * (joint2->Position() + joint0->Position()) +
                            0.5f * dist * h);
      pole = c2.get();
    }
  }
}

// -------------------------- Helpers --------------------------

void twoboneikDragableTarget(Entity **joint, std::vector<char> &namebuffer,
                             std::string label) {
  std::strcpy(namebuffer.data(),
              *joint == nullptr ? "" : (*joint)->name.c_str());
  GUIUtils::DragableEntityTarget(("##" + label).c_str(), label,
                                 [&](Entity *entity) {
                                   *joint = entity;
                                   return true;
                                 },
                                 namebuffer, [&]() { *joint = nullptr; });
}

}; // namespace aEngine

REGISTER_SCRIPT_SL(aEngine, TwoBoneIK)