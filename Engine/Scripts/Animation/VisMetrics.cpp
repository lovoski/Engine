#include "Scripts/Animation/VisMetrics.hpp"

namespace aEngine {

struct ScrollingBuffer {
  int MaxSize;
  int Offset;
  ImVector<ImVec2> Data;
  ScrollingBuffer(int max_size = 2000) {
    MaxSize = max_size;
    Offset = 0;
    Data.reserve(MaxSize);
  }
  void AddPoint(float x, float y) {
    if (Data.size() < MaxSize)
      Data.push_back(ImVec2(x, y));
    else {
      Data[Offset] = ImVec2(x, y);
      Offset = (Offset + 1) % MaxSize;
    }
  }
  void Erase() {
    if (Data.size() > 0) {
      Data.shrink(0);
      Offset = 0;
    }
  }
};

bool equalIgnoreCase(char a, char b) {
  if (a >= 'A' && a <= 'Z')
    a = a - 'A' + 'a';
  if (b >= 'A' && b <= 'Z')
    b = b - 'A' + 'a';
  return a == b;
}

bool isSubstr(std::string source, std::string pattern) {
  if (source.size() < pattern.size())
    return false;
  int p = 0;
  for (int i = 0; i <= source.size() - pattern.size(); ++i) {
    if (equalIgnoreCase(source[i], pattern[0])) {
      bool match = true;
      for (int j = 0; j < pattern.size(); ++j) {
        if (!equalIgnoreCase(source[i + j], pattern[j])) {
          match = false;
          break;
        }
      }
      if (match)
        return true;
    }
  }
  return false;
}

void VisMetrics::locateHumanoidFoot(std::shared_ptr<Animator> animator) {
  std::vector<std::string> candidates{"toe"};
  if (animator == nullptr) {
    lFoot = nullptr;
    rFoot = nullptr;
    return;
  }
  if (lFoot && rFoot)
    return;
  for (auto candidate : candidates) {
    auto left = "left" + candidate;
    auto right = "right" + candidate;
    for (auto ele : animator->SkeletonMap) {
      auto jointName = animator->actor->jointNames[ele.second.actorInd];
      auto lMatch = isSubstr(jointName, left);
      auto rMatch = isSubstr(jointName, right);
      if (lMatch)
        lFoot = ele.second.joint;
      if (rMatch)
        rFoot = ele.second.joint;
      if (lFoot && rFoot)
        return;
    }
  }
}

float VisMetrics::slideMetricsCurrentPose(std::shared_ptr<Animator> animator) {
  auto sm = animator->SkeletonMap;
  std::vector<glm::vec3> currentPositions;
  for (auto ele : sm) {
    currentPositions.push_back(ele.second.joint->Position());
  }
  if (prevPositions.size() == 0)
    prevPositions = currentPositions;
  return Animation::SlideMetrics(prevPositions, currentPositions,
                                 heightThreshold);
}

void VisMetrics::DrawToScene() {
  EntityID camera;
  if (GWORLD.GetActiveCamera(camera)) {
    glDisable(GL_DEPTH_TEST);
    auto cameraComp = GWORLD.GetComponent<Camera>(camera);
    if (showContactJoint) {
      for (auto contactJoint : contactJoints) {
        VisUtils::DrawWireSphere(contactJoint->Position(), cameraComp->VP, 1.0f,
                                 glm::vec3(1.0f, 0.0f, 0.0f));
      }
    }
    glEnable(GL_DEPTH_TEST);
  }
}

// Compute the number of self-intersecting triangles in this mesh
// with the help of bvh.
int computeMeshSelfIntersection(std::shared_ptr<Mesh> mesh,
                                float offset = 0.01f) {
  std::vector<std::pair<int, int>> hit;
  auto transform = GWORLD.EntityFromID(mesh->GetID())->GlobalTransformMatrix();
  if (!mesh->AsCollider)
    mesh->BuildBVH(transform);

  // build another bvh with offset mesh
  Spatial::BVH bvh;
  std::vector<Vertex> vertices;
  if (mesh->Deformed) {
    auto bufferSize = sizeof(Vertex) * mesh->GetMeshInstance()->vertices.size();
    mesh->target.BindAs(GL_SHADER_STORAGE_BUFFER);
    Vertex *mappedMemory = (Vertex *)glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER, 0, bufferSize, GL_MAP_READ_BIT);
    if (mappedMemory != nullptr) {
      vertices.reserve(mesh->GetMeshInstance()->vertices.size());
      for (int i = 0; i < mesh->GetMeshInstance()->vertices.size(); ++i) {
        vertices.push_back(mappedMemory[i]);
      }
      glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
      mesh->target.UnbindAs(GL_SHADER_STORAGE_BUFFER);
    } else {
      LOG_F(ERROR, "memory mapped failed");
    }
  } else {
    // transform all vertices
    auto vertCount = mesh->GetMeshInstance()->vertices.size();
    vertices.reserve(vertCount);
    for (int i = 0; i < vertCount; ++i) {
      vertices.push_back(mesh->GetMeshInstance()->vertices[i]);
      vertices[i].Position = transform * vertices[i].Position;
    }
  }
  if (vertices.size() > 0) {
    std::vector<Spatial::Triangle> triangles;
    for (int fi = 0; fi < mesh->GetMeshInstance()->indices.size() / 3; ++fi) {
      Spatial::Triangle tri;
      glm::vec3 v0 =
          vertices[mesh->GetMeshInstance()->indices[3 * fi + 0]].Position;
      glm::vec3 v1 =
          vertices[mesh->GetMeshInstance()->indices[3 * fi + 1]].Position;
      glm::vec3 v2 =
          vertices[mesh->GetMeshInstance()->indices[3 * fi + 2]].Position;
      auto normal = Math::FaceNormal(v0, v1, v2);
      tri.V = {v0 + offset * normal, v1 + offset * normal,
               v2 + offset * normal};
      triangles.push_back(tri);
    }
    bvh.SetAllPrimitives(triangles);

    mesh->bvh.Intersect(bvh, hit);
  } else {
    LOG_F(ERROR,
          "offset mesh not exist, can't perform self intersection check");
  }

  return hit.size();
}

void VisMetrics::drawCustomInspectorGUI() {
  auto animator = entity->GetComponent<Animator>();
  ImGui::MenuItem("Slide Metrics", nullptr, nullptr, false);
  ImGui::TextWrapped("Value: %.3f", animator == nullptr
                                        ? -1
                                        : slideMetricsCurrentPose(animator));

  // better visualization with implot
  static float history = 5.0f;
  if (ImPlot::BeginPlot("Height##vismetrics", {-1, 200})) {
    static ImPlotAxisFlags flags = ImPlotAxisFlags_NoTickLabels;
    static float time = 0;
    static ScrollingBuffer lFootData, rFootData, thresholdData;
    static std::string lFootName = "", rFootName = "";
    time += ImGui::GetIO().DeltaTime;
    locateHumanoidFoot(animator);
    lFootData.AddPoint(time, lFoot == nullptr ? 0.0f : lFoot->Position().y);
    rFootData.AddPoint(time, rFoot == nullptr ? 0.0f : rFoot->Position().y);
    thresholdData.AddPoint(time, heightThreshold);
    ImPlot::SetupAxes(nullptr, nullptr, flags, flags);
    ImPlot::SetupAxisLimits(ImAxis_X1, time - history, time, ImGuiCond_Always);
    ImPlot::SetupAxisLimits(ImAxis_Y1, -0.5f, 25.0f);
    ImPlot::SetNextFillStyle(IMPLOT_AUTO_COL, 0.5f);
    ImPlot::PlotLine("Left Foot", &lFootData.Data[0].x, &lFootData.Data[0].y,
                     lFootData.Data.size(), 0, lFootData.Offset,
                     2 * sizeof(float));
    ImPlot::PlotLine("Right Foot", &rFootData.Data[0].x, &rFootData.Data[0].y,
                     rFootData.Data.size(), 0, rFootData.Offset,
                     2 * sizeof(float));
    ImPlot::PlotLine("Threshold", &thresholdData.Data[0].x,
                     &thresholdData.Data[0].y, thresholdData.Data.size(), 0,
                     thresholdData.Offset, 2 * sizeof(float));
    // EndPlot should only gets called when BeginPlot is true
    ImPlot::EndPlot();
  }

  ImGui::Checkbox("Show Contact##vismetrics", &showContactJoint);
  ImGui::SliderFloat("Threshold##vismetrics", &heightThreshold, 0.0f, 20.0f);
  ImGui::SliderFloat("History##vismetrics", &history, 0.0f, 10.0f);

  // self intersection metrics
  ImGui::Separator();
  ImGui::MenuItem("Self Intersection", nullptr, nullptr, false);
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
        selfIntersectionValue =
            computeMeshSelfIntersection(meshBase->GetComponent<Mesh>());
        LOG_F(INFO, "compute self intersection for mesh on entity \"%s\", found %d hits",
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

void VisMetrics::Update(float dt) {
  contactJoints.clear();
  auto animator = entity->GetComponent<Animator>();
  if (animator) {
    for (auto ele : animator->SkeletonMap) {
      if (ele.second.joint->Position().y < heightThreshold) {
        contactJoints.push_back(ele.second.joint);
      }
    }
  }
}

}; // namespace aEngine