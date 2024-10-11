#include "Scripts/Geometry/SelfIntersection.hpp"

namespace aEngine {

void SelfIntersection::DrawToScene() {
  EntityID camera;
  if (GWORLD.GetActiveCamera(camera)) {
    glDisable(GL_DEPTH_TEST);
    auto cameraComp = GWORLD.GetComponent<Camera>(camera);
    auto vp = cameraComp->VP;
    auto viewport = GWORLD.Context.sceneWindowSize;
    if (showIntersectingTriangles) {
      VisUtils::DrawSquares(intersectingTriangles, intersectionVisSize, vp,
                            viewport, glm::vec3(1.0f, 0.0f, 0.0f));
    }
    glEnable(GL_DEPTH_TEST);
  }
}

// Compute the number of self-intersecting triangles in this mesh
// with the help of bvh.
int SelfIntersection::computeMeshSelfIntersection(std::shared_ptr<Mesh> mesh,
                                                  float offset) {
  intersectingTriangles.clear();
  std::vector<std::pair<int, int>> hit;
  auto transform = GWORLD.EntityFromID(mesh->GetID())->GlobalTransformMatrix();
  if (!mesh->AsCollider)
    mesh->BuildBVH(transform);

  // build another bvh with offset mesh
  Spatial::BVH bvh = mesh->bvh;
  for (auto &tri : bvh.Primitives) {
    auto normal = Math::FaceNormal(tri.V[0], tri.V[1], tri.V[2]);
    tri.V[0] += offset * normal;
    tri.V[1] += offset * normal;
    tri.V[2] += offset * normal;
  }
  mesh->bvh.Intersect(bvh, hit);

  for (auto &p : hit) {
    intersectingTriangles.push_back(mesh->bvh.Primitives[p.first].Barycenter());
    intersectingTriangles.push_back(bvh.Primitives[p.second].Barycenter());
  }

  return hit.size();
}

void SelfIntersection::DrawInspectorGUI() {
  // self intersection metrics
  ImGui::MenuItem("Self Intersection", nullptr, nullptr, false);
  ImGui::Checkbox("Show Intersections", &showIntersectingTriangles);
  ImGui::DragFloat("Offset", &intersectionOffset, 0.0001f, 0.0f, 1.0f);
  ImGui::SliderFloat("##Visualize Size", &intersectionVisSize, 0.0f, 5.0f);
  static int selfIntersectionValue = -1;
  ImGui::Text("Intersection Count: %d", selfIntersectionValue);
  ImGui::BeginChild("chooseselfintersectionmeshbase", {-1, 30});

  static char meshBaseEntityName[200] = {0};
  sprintf(meshBaseEntityName, meshBaseForIntersection == nullptr
                                  ? ""
                                  : meshBaseForIntersection->name.c_str());
  ImGui::InputTextWithHint("##selfintersectionmeshbase",
                           "Entity Having Mesh Component", meshBaseEntityName,
                           sizeof(meshBaseEntityName),
                           ImGuiInputTextFlags_ReadOnly);
  ImGui::SameLine();
  if (ImGui::Button("Clear##selfintersection", {-1, -1})) {
    for (int i = 0; i < sizeof(meshBaseEntityName); ++i)
      meshBaseEntityName[i] = 0;
  }
  ImGui::EndChild();
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("ENTITYID_DATA")) {
      Entity *meshBase = *(Entity **)payload->Data;
      if (meshBase && meshBase->HasComponent<Mesh>()) {
        meshBaseForIntersection = meshBase;
        selfIntersectionValue = computeMeshSelfIntersection(
            meshBase->GetComponent<Mesh>(), intersectionOffset);
        LOG_F(INFO,
              "compute self intersection for mesh on entity \"%s\", found %d "
              "hits",
              meshBase->name.c_str(), selfIntersectionValue);
      } else {
        LOG_F(ERROR, "the entiy don't have mesh component, can't be used to "
                     "compute self intersection");
        meshBaseForIntersection = nullptr;
      }
    }
    ImGui::EndDragDropTarget();
  }
}

}; // namespace aEngine

REGISTER_SCRIPT(aEngine, SelfIntersection)