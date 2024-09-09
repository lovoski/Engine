#include "Component/Mesh.hpp"

namespace aEngine {

Mesh::Mesh(EntityID id, Render::Mesh *mesh) : BaseComponent(id) {
  SetupMesh(mesh);
}

void Mesh::SetupMesh(Render::Mesh *mesh) {
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
    LOG_F(WARNING, "mesh is null, can't setup vao for Mesh component");
  }
}

void BuildSpatialDS(Render::Mesh *mesh) {}

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
  static char inputBuf[200]{0};
  if (meshInstance)
    sprintf(inputBuf, meshInstance->identifier.c_str());
  else
    sprintf(inputBuf, "");
  ImGui::PushItemWidth(-1);
  ImGui::InputTextWithHint("##identifier", "Mesh Identifier", inputBuf,
                           sizeof(inputBuf), ImGuiInputTextFlags_ReadOnly);
  ImGui::PopItemWidth();
  // if (ImGui::BeginDragDropTarget()) {
  //   if (const ImGuiPayload *payload =
  //           ImGui::AcceptDragDropPayload("LOADED_ASSET")) {
  //     char *asset = (char *)payload->Data;
  //   }
  //   ImGui::EndDragDropTarget();
  // }
  ImGui::Separator();
  if (ImGui::BeginTable("Properties##meshdataproperties", 2)) {
    ImGui::TableSetupColumn("Field");
    ImGui::TableSetupColumn("Number");
    ImGui::TableHeadersRow();

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

    ImGui::EndTable();
  }
}

}; // namespace aEngine