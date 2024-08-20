#include "Function/General/Deformers.hpp"

namespace aEngine {

void SkinnedMeshDeformer::DeformMesh(Entity *entity) {
  if (entity->HasComponent<Animator>()) {
    // only update the mesh if the entity has animator component
    auto &animator = entity->GetComponent<Animator>();
    cs->Use();
    mesh->defaultStates.BindToPointAs(GL_SHADER_STORAGE_BUFFER, 0);
    mesh->skeletonTransforms.SetDataAs(GL_SHADER_STORAGE_BUFFER,
                                       animator.GetSkeletonTransforms());
    mesh->skeletonTransforms.BindToPointAs(GL_SHADER_STORAGE_BUFFER, 1);
    cs->Dispatch(8, 8, 0);
  }
}

void SkinnedMeshDeformer::DrawInspectorGUI() { drawInspectorGUIDefault(); }

}; // namespace aEngine