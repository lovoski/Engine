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
    float dist = glm::length(pt - p0);
    auto dir = glm::normalize(pt - p0);
    auto normal =
        glm::normalize(glm::cross(glm::normalize(pole->Position() - p0),
                                  glm::normalize(p2 - pole->Position())));
    if (glm::abs(dist - l01 - l12) < 1e-3f) {
      p1 = p0 + l01 * dir;
      p2 = p0 + (l01 + l12) * dir;
    } else {
      auto h = glm::normalize(glm::cross(dir, normal));
      float cosalpha = glm::clamp((l01 * l01 + dist * dist - l12 * l12) /
                                      (2.0f * l01 * dist),
                                  0.0f, 1.0f);
      p1 = glm::sqrt(glm::abs(1.0f - cosalpha * cosalpha)) * l01 * h +
           cosalpha * l01 * dir + p0;
      p2 = pt;
    }
    // joint0->SetGlobalRotation(glm::quatLookAt(
    //     glm::normalize(glm::cross(glm::normalize(p1 - p0), normal)),
    //     glm::normalize(p1 - p0)));
    // joint1->SetGlobalRotation(glm::quatLookAt(
    //     glm::normalize(glm::cross(glm::normalize(p2 - p1), normal)),
    //     glm::normalize(p2 - p1)));
    joint2->SetGlobalRotation(target->Rotation());
  }
}

void TwoBoneIK::DrawToScene() {
  EntityID camera;
  if (GWORLD.GetActiveCamera(camera)) {
    auto vp = GWORLD.GetComponent<Camera>(camera)->VP;
    if (target)
      VisUtils::DrawWireSphere(target->Position(), vp, visRadius,
                               VisUtils::Red);
    if (pole)
      VisUtils::DrawWireSphere(pole->Position(), vp, visRadius,
                               VisUtils::Yellow);
  }
}

void TwoBoneIK::DrawInspectorGUI() {
  ImGui::MenuItem("Objects", nullptr, nullptr, false);
  twoboneikDragableTarget(&joint0, j0NameBuffer, "Joint 0");
  twoboneikDragableTarget(&joint1, j1NameBuffer, "Joint 1");
  twoboneikDragableTarget(&joint2, j2NameBuffer, "Joint 2");
  twoboneikDragableTarget(&pole, poleNameBuffer, "Pole Object");
  twoboneikDragableTarget(&target, targetNameBuffer, "Target Object");

  if (joint0 && joint1)
    l01 = glm::length(joint0->Position() - joint1->Position());
  if (joint1 && joint2)
    l12 = glm::length(joint2->Position() - joint1->Position());

  ImGui::Separator();
  if (ImGui::Button("Create Target and Pole", {-1, 30})) {
    if (joint0 && joint1 && joint2) {
      auto c1 = GWORLD.AddNewEntity();
      c1->SetGlobalPosition(joint2->Position());
      c1->SetGlobalRotation(joint2->Rotation());
      target = c1.get();
      auto c2 = GWORLD.AddNewEntity();
      auto p0 = joint0->Position(), p1 = joint1->Position(),
           p2 = joint2->Position();
      auto forward = glm::normalize(p1 - p0 -
                                    glm::dot(p1 - p0, glm::normalize(p2 - p0)) *
                                        glm::normalize(p2 - p0));
      c2->SetGlobalPosition(0.5f * (p0 + p2) + forward);
      pole = c2.get();
    }
  }
  ImGui::DragFloat("Vis Size", &visRadius, 0.01f, 0.0f, 1.0f);
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