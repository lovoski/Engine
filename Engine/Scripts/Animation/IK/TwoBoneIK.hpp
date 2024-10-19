#pragma once

#include "API.hpp"

namespace aEngine {

class TwoBoneIK : public Scriptable {
public:
  TwoBoneIK() {}
  ~TwoBoneIK() {}

  void Update(float dt) override;
  void LateUpdate(float dt) override;
  void DrawToScene() override;
  void DrawInspectorGUI() override;

  std::string getInspectorWindowName() override { return "Two Bone IK"; };

  template <typename Archive> void save(Archive &ar) const {
    ar(start == nullptr ? 0 : start->ID, mid == nullptr ? 0 : mid->ID,
       end == nullptr ? 0 : end->ID, pole == nullptr ? 0 : pole->ID);
  }
  template <typename Archive> void load(Archive &ar) {
    EntityID id0, id1, id2, id3;
    ar(id0, id1, id2, id3);
    start = id0 == 0 ? nullptr : GWORLD.EntityFromID(id0).get();
    mid = id1 == 0 ? nullptr : GWORLD.EntityFromID(id1).get();
    end = id2 == 0 ? nullptr : GWORLD.EntityFromID(id2).get();
    pole = id3 == 0 ? nullptr : GWORLD.EntityFromID(id3).get();
  }

private:
  Entity *start = nullptr, *mid = nullptr, *end = nullptr, *pole = nullptr;
};

}; // namespace aEngine