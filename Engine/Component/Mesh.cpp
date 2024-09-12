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

void Mesh::SetupBVH(glm::mat4 &transform) {
  // initialize bvh
  auto &indices = meshInstance->indices;
  auto &vertices = meshInstance->vertices;
  std::vector<Spatial::Triangle> triangles;
  for (int fi = 0; fi < meshInstance->indices.size() / 3; ++fi) {
    Spatial::Triangle tri;
    tri.V = {transform * vertices[indices[3 * fi + 0]].Position,
             transform * vertices[indices[3 * fi + 1]].Position,
             transform * vertices[indices[3 * fi + 2]].Position};
    triangles.push_back(tri);
  }
  bvh.SetAllPrimitives(triangles);
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
    ImGui::Text(meshInstance->identifier.c_str());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Vertices");
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("%d", meshInstance->vertices.size());

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Faces");
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("%d", meshInstance->indices.size() / 3);

    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Collider");
    ImGui::TableSetColumnIndex(1);
    ImGui::Checkbox("##asmeshcollider", &AsCollider);

    ImGui::EndTable();
  }
  if (ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("ASSET_FILENAME")) {
      char *asset = (char *)payload->Data;
    }
    ImGui::EndDragDropTarget();
  }
}

}; // namespace aEngine