#include "Component/Mesh.hpp"

namespace aEngine {

Mesh::Mesh(EntityID id, Render::Mesh *mesh)
    : BaseComponent(id), meshInstance(mesh) {
  if (meshInstance == nullptr) {
    LOG_F(ERROR, "Mesh component can't process null mesh instance, use cube "
                 "primitive by default");
    meshInstance = Loader.GetMesh("::cubePrimitive", "");
  }
  SetMeshInstance(meshInstance);
}

void Mesh::SetMeshInstance(Render::Mesh *mesh) {
  if (mesh) {
    vao.Bind();
    mesh->vbo.BindAs(GL_ARRAY_BUFFER);
    mesh->ebo.BindAs(GL_ELEMENT_ARRAY_BUFFER);
    vao.LinkAttrib(mesh->vbo, 0, 4, GL_FLOAT, sizeof(Vertex), (void *)0);
    vao.LinkAttrib(mesh->vbo, 1, 4, GL_FLOAT, sizeof(Vertex),
                   (void *)(offsetof(Vertex, Normal)));
    vao.LinkAttrib(mesh->vbo, 2, 4, GL_FLOAT, sizeof(Vertex),
                   (void *)(offsetof(Vertex, TexCoords)));
    vao.LinkAttrib(mesh->vbo, 3, 4, GL_FLOAT, sizeof(Vertex),
                   (void *)(offsetof(Vertex, Color))); // vertex color
    vao.Unbind();
    mesh->vbo.UnbindAs(GL_ARRAY_BUFFER);
    mesh->ebo.UnbindAs(GL_ELEMENT_ARRAY_BUFFER);

    target.SetDataAs(GL_SHADER_STORAGE_BUFFER, mesh->vertices);
    target.UnbindAs(GL_SHADER_STORAGE_BUFFER);

    meshInstance = mesh;
  } else {
    LOG_F(ERROR,
          "trying to set null mesh instance to mesh component, do nothing.");
  }
}

void Mesh::buildBVHDeformed() {
  // map memory from `target` back to cpu
  std::vector<Vertex> vertices;
  auto bufferSize = sizeof(Vertex) * meshInstance->vertices.size();
  target.BindAs(GL_SHADER_STORAGE_BUFFER);
  Vertex *mappedMemory = (Vertex *)glMapBufferRange(
      GL_SHADER_STORAGE_BUFFER, 0, bufferSize, GL_MAP_READ_BIT);
  if (mappedMemory != nullptr) {
    vertices.reserve(meshInstance->vertices.size());
    for (int i = 0; i < meshInstance->vertices.size(); ++i) {
      vertices.push_back(mappedMemory[i]);
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    target.UnbindAs(GL_SHADER_STORAGE_BUFFER);
    std::vector<Spatial::Triangle> triangles;
    for (int fi = 0; fi < meshInstance->indices.size() / 3; ++fi) {
      Spatial::Triangle tri;
      tri.V = {vertices[meshInstance->indices[3 * fi + 0]].Position,
               vertices[meshInstance->indices[3 * fi + 1]].Position,
               vertices[meshInstance->indices[3 * fi + 2]].Position};
      triangles.push_back(tri);
    }
    bvh.SetAllPrimitives(triangles);
  } else {
    LOG_F(ERROR, "memory mapped failed");
  }
}

void Mesh::buildBVHOriginal(glm::mat4 transform) {
  std::vector<Spatial::Triangle> triangles;
  auto &vertices = meshInstance->vertices;
  for (int fi = 0; fi < meshInstance->indices.size() / 3; ++fi) {
    Spatial::Triangle tri;
    tri.V = {transform * vertices[meshInstance->indices[3 * fi + 0]].Position,
             transform * vertices[meshInstance->indices[3 * fi + 1]].Position,
             transform * vertices[meshInstance->indices[3 * fi + 2]].Position};
    triangles.push_back(tri);
  }
  bvh.SetAllPrimitives(triangles);
}

void Mesh::BuildBVH(glm::mat4 &transform) {
  // initialize bvh
  if (Deformed)
    buildBVHDeformed();
  else
    buildBVHOriginal(transform);
}

void Mesh::Bind() {
  vao.Bind();
  if (Deformed) {
    // use `target` as vbo
    target.BindAs(GL_ARRAY_BUFFER);
    vao.LinkAttrib(target, 0, 4, GL_FLOAT, sizeof(Vertex), (void *)0);
    vao.LinkAttrib(target, 1, 4, GL_FLOAT, sizeof(Vertex),
                   (void *)(offsetof(Vertex, Normal)));
    vao.LinkAttrib(target, 2, 4, GL_FLOAT, sizeof(Vertex),
                   (void *)(offsetof(Vertex, TexCoords)));
    vao.LinkAttrib(target, 3, 4, GL_FLOAT, sizeof(Vertex),
                   (void *)(offsetof(Vertex, Color))); // vertex color
  }
}

void Mesh::Unbind() {
  vao.Unbind();
  if (Deformed)
    target.UnbindAs(GL_ARRAY_BUFFER);
}

void Mesh::DrawInspectorGUI() {
  ImGui::Separator();
  if (ImGui::BeginTable("Properties##meshdataproperties", 2)) {
    ImGui::TableSetupColumn("Field");
    ImGui::TableSetupColumn("Value");
    ImGui::TableHeadersRow();

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Identifier");
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("%s", meshInstance->identifier.c_str());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Vertices");
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("%ld", meshInstance->vertices.size());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Faces");
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("%ld", meshInstance->indices.size() / 3);

    ImGui::EndTable();
  }
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("ASSET_FILENAME")) {
      char *asset = (char *)payload->Data;
    }
    ImGui::EndDragDropTarget();
  }

  if (ImGui::CollapsingHeader("Mesh Collider")) {
    if (ImGui::Checkbox("Enable", &AsCollider)) {
      if (AsCollider) {
        LOG_F(INFO, "build bvh on enable mesh collider");
        auto transform = GWORLD.EntityFromID(entityID)->GlobalTransformMatrix();
        BuildBVH(transform);
      }
    }
    ImGui::Separator();
    if (!AsCollider)
      ImGui::BeginDisabled();
    ImGui::Checkbox("Static Collider", &StaticCollider);
    ImGui::Checkbox("Draw Leaf Only", &DrawLeafNodeOnly);
    if (ImGui::Button("Build", {-1, 30})) {
      auto transform = GWORLD.EntityFromID(entityID)->GlobalTransformMatrix();
      BuildBVH(transform);
    }
    if (ImGui::BeginTable("Properties##meshbvhdata", 2)) {
      ImGui::TableSetupColumn("Field");
      ImGui::TableSetupColumn("Value");
      ImGui::TableHeadersRow();

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("Num Nodes");
      ImGui::TableSetColumnIndex(1);
      ImGui::Text("%d", bvh.GetNumNodes());

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("Num Leaf Nodes");
      ImGui::TableSetColumnIndex(1);
      ImGui::Text("%ld", bvh.LeafNodes().size());

      ImGui::TableNextRow();
      ImGui::TableSetColumnIndex(0);
      ImGui::Text("Num Primitives");
      ImGui::TableSetColumnIndex(1);
      ImGui::Text("%ld", bvh.Primitives.size());

      ImGui::EndTable();
    }
    if (!AsCollider)
      ImGui::EndDisabled();
  }
}

}; // namespace aEngine

REGISTER_COMPONENT(aEngine, Mesh);