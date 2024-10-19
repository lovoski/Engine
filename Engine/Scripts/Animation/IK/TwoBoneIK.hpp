#pragma once

#include "API.hpp"

namespace aEngine {

class TwoBoneIK : public Scriptable {
public:
  TwoBoneIK() {}
  ~TwoBoneIK() {}

  void LateUpdate(float dt) override;
  void DrawToScene() override;
  void DrawInspectorGUI() override;

  std::string getInspectorWindowName() override { return "Two Bone IK"; };

  template <typename Archive> void save(Archive &ar) const {
    ar(joint0 == nullptr ? 0 : joint0->ID, joint1 == nullptr ? 0 : joint1->ID,
       joint2 == nullptr ? 0 : joint2->ID, pole == nullptr ? 0 : pole->ID,
       target == nullptr ? 0 : target->ID);
  }
  template <typename Archive> void load(Archive &ar) {
    EntityID i0, i1, i2, i3, i4;
    ar(i0, i1, i2, i3, i4);
    joint0 = i0 == 0 ? nullptr : GWORLD.EntityFromID(i0).get();
    joint1 = i1 == 0 ? nullptr : GWORLD.EntityFromID(i1).get();
    joint2 = i2 == 0 ? nullptr : GWORLD.EntityFromID(i2).get();
    pole = i3 == 0 ? nullptr : GWORLD.EntityFromID(i3).get();
    target = i4 == 0 ? nullptr : GWORLD.EntityFromID(i4).get();
  }

private:
  std::vector<char> j0NameBuffer = std::vector<char>(200),
                    j1NameBuffer = std::vector<char>(200),
                    j2NameBuffer = std::vector<char>(200),
                    poleNameBuffer = std::vector<char>(200),
                    targetNameBuffer = std::vector<char>(200);
  Entity *joint0 = nullptr, *joint1 = nullptr, *joint2 = nullptr;
  Entity *pole = nullptr, *target = nullptr;
};

}; // namespace aEngine