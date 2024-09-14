#pragma once

#include "API.hpp"

namespace aEngine {

class SelfIntersection : public Scriptable {
public:
  void DrawInspectorGUI() override;

  void DrawToScene() override;

private:
  Entity *meshBaseForIntersection = nullptr;
  bool showIntersectingTriangles = true;
  float intersectionVisSize = 0.5f;
  float intersectionOffset = 0.01f;
  std::vector<glm::vec3> intersectingTriangles;

  std::string getInspectorWindowName() override { return "Self Intersection"; }

  int computeMeshSelfIntersection(std::shared_ptr<Mesh> mesh,
                                  float offset = 0.01f);
};

}; // namespace aEngine