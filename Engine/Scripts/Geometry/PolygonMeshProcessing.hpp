/**
 * This script use the built in algorithms in CGAL to process polygon mesh. The
 * script requires the entity posessing it have a Mesh component.
 */
#pragma once

#include "API.hpp"

namespace aEngine {

class PolygonMeshProcessing : public Scriptable {
public:
  PolygonMeshProcessing() {}
  ~PolygonMeshProcessing() {}

  void Update(float dt) override;
  void LateUpdate(float dt) override;

  void DrawInspectorGUI() override;
  void DrawToScene() override;

private:
  std::string getInspectorWindowName() { return "Polygon Mesh Processing"; }
};

}; // namespace aEngine