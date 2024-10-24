#include "Function/AssetsLoader.hpp"
#include "System/Animation/AnimationSystem.hpp"

#include <ufbx.h>

using std::string;
using std::vector;

namespace aEngine {

Render::Mesh *ProcessMesh(ufbx_mesh *mesh, ufbx_mesh_part &part,
                          std::map<string, std::size_t> &boneMapping,
                          std::string modelPath);

struct BoneInfo {
  std::string boneName;
  ufbx_node *node;
  glm::vec3 localPosition = glm::vec3(0.0f);
  glm::quat localRotation = glm::quat(1.0f, glm::vec3(0.0f));
  glm::vec3 localScale = glm::vec3(1.0f);
  glm::mat4 offsetMatrix = glm::mat4(1.0f);
  int parentIndex =
      -1; // Index of the parent bone in the hierarchy (-1 if root)
  std::vector<int> children = std::vector<int>(); // Indices of child bones
};

struct KeyFrame {
  glm::vec3 localPosition;
  glm::quat localRotation;
  glm::vec3 localScale;
};

glm::vec3 ConvertToGLM(ufbx_vec3 &v) { return {v.x, v.y, v.z}; }
glm::vec4 ConvertToGLM(ufbx_vec4 &v) { return {v.x, v.y, v.z, v.w}; }
glm::quat ConvertToGLM(ufbx_quat &q) { return glm::quat(q.w, q.x, q.y, q.z); }

void AssetsLoader::loadFBXModelFile(std::vector<Render::Mesh *> &meshes,
                                    std::string modelPath) {
  ufbx_load_opts opts = {};
  opts.target_axes = ufbx_axes_right_handed_y_up;
  opts.target_unit_meters = 1.0f;
  ufbx_error error;
  ufbx_scene *scene = ufbx_load_file(modelPath.c_str(), &opts, &error);
  if (!scene) {
    LOG_F(ERROR, "failed to load scene: %s", error.description.data);
    return;
  }

  // the parent bone will always have lower index in globalBones
  // than all its children
  std::vector<BoneInfo> globalBones;
  std::map<std::string, std::size_t> boneMapping;
  std::map<int, int> oldBoneInd2NewInd;
  std::stack<ufbx_node *> s;
  s.push(scene->root_node);
  while (!s.empty()) {
    auto cur = s.top();
    s.pop();
    if (cur->bone) {
      // if this is a bone node
      string boneName = cur->name.data;
      if (boneMapping.find(boneName) == boneMapping.end()) {
        int currentBoneInd = globalBones.size();
        boneMapping.insert(std::make_pair(boneName, currentBoneInd));
        BoneInfo bi;
        bi.boneName = boneName;
        bi.node = cur;
        auto localTransform = cur->local_transform;
        bi.localScale = ConvertToGLM(localTransform.scale);
        bi.localPosition = ConvertToGLM(localTransform.translation);
        bi.localRotation = ConvertToGLM(localTransform.rotation);
        // find a parent bone
        auto curParent = cur->parent;
        while (curParent) {
          if (curParent->bone) {
            auto parentIt = boneMapping.find(curParent->name.data);
            if (parentIt == boneMapping.end()) {
              LOG_F(ERROR, "parent bone don't exist in boneMapping");
            } else {
              bi.parentIndex = parentIt->second;
              globalBones[parentIt->second].children.push_back(currentBoneInd);
              break;
            }
          }
          curParent = curParent->parent;
        }
        globalBones.push_back(bi);
      }
    }
    for (auto child : cur->children) {
      s.push(child);
    }
  }

  if (globalBones.size() > 1 && boneMapping.size() > 1) {
    int animStackCount = scene->anim_stacks.count;
    // find the longest animation to import
    double longestDuration = -1.0;
    int longestAnimInd = -1;
    for (int animInd = 0; animInd < animStackCount; ++animInd) {
      auto tmpAnim = scene->anim_stacks[animInd]->anim;
      auto duration = tmpAnim->time_end - tmpAnim->time_begin;
      if (duration > longestDuration) {
        duration = longestDuration;
        longestAnimInd = animInd;
      }
    }
    std::vector<std::vector<KeyFrame>> animationPerJoint(globalBones.size(),
                                                         vector<KeyFrame>());
    if (longestAnimInd != -1) {
      auto anim = scene->anim_stacks[longestAnimInd]
                      ->anim; // import the active animation only
      auto animSystem = GWORLD.GetSystemInstance<AnimationSystem>();
      auto startTime = anim->time_begin, endTime = anim->time_end;
      double sampleDelta = 1.0 / animSystem->SystemFPS;
      for (auto jointInd = 0; jointInd < globalBones.size(); ++jointInd) {
        auto jointNode = globalBones[jointInd].node;
        for (double currentTime = startTime; currentTime < endTime;
             currentTime += sampleDelta) {
          auto localTransform =
              ufbx_evaluate_transform(anim, jointNode, currentTime);
          KeyFrame kf;
          kf.localPosition = ConvertToGLM(localTransform.translation);
          kf.localRotation = ConvertToGLM(localTransform.rotation);
          kf.localScale = ConvertToGLM(localTransform.scale);
          animationPerJoint[jointInd].push_back(kf);
        }
      }
    }

    // create skeleton, register it in allSkeletons
    Animation::Skeleton *skel = new Animation::Skeleton();
    // map indices from scene->skin_clusters to globalBones
    for (int clusterInd = 0; clusterInd < scene->skin_clusters.count;
         ++clusterInd) {
      auto cluster = scene->skin_clusters[clusterInd];
      std::string name = cluster->bone_node->name.data;
      auto it = boneMapping.find(name);
      if (it == boneMapping.end()) {
        LOG_F(ERROR, "cluster %s doesn't appear in globalBones", name.c_str());
      } else {
        // update offset matrix of this bone
        auto offsetMatrix = cluster->geometry_to_bone;
        globalBones[it->second].offsetMatrix =
            glm::mat4(glm::vec4(ConvertToGLM(offsetMatrix.cols[0]), 0.0f),
                      glm::vec4(ConvertToGLM(offsetMatrix.cols[1]), 0.0f),
                      glm::vec4(ConvertToGLM(offsetMatrix.cols[2]), 0.0f),
                      glm::vec4(ConvertToGLM(offsetMatrix.cols[3]), 1.0f));
        oldBoneInd2NewInd.insert(std::make_pair(static_cast<int>(clusterInd),
                                                static_cast<int>(it->second)));
      }
    }
    // assume one file only contains one skeleton
    allSkeletons.insert(std::make_pair(modelPath, skel));

    // setup variables in the skeleton
    skel->skeletonName = "armature";
    skel->path = modelPath;
    for (int jointInd = 0; jointInd < globalBones.size(); ++jointInd) {
      skel->jointNames.push_back(globalBones[jointInd].boneName);
      skel->jointParent.push_back(globalBones[jointInd].parentIndex);
      skel->jointChildren.push_back(globalBones[jointInd].children);
      skel->jointOffset.push_back(globalBones[jointInd].localPosition);
      skel->jointRotation.push_back(globalBones[jointInd].localRotation);
      skel->jointScale.push_back(globalBones[jointInd].localScale);
      skel->offsetMatrices.push_back(globalBones[jointInd].offsetMatrix);
    }
    // create motion for the skeleton
    int numFrames = animationPerJoint[0].size();
    int numJoints = animationPerJoint.size();
    if (numFrames > 0) {
      Animation::Motion *motion = new Animation::Motion();
      motion->fps = 30; // use 30fps by default
      motion->skeleton = *skel;
      motion->path = modelPath;
      allMotions.insert(std::make_pair(modelPath, motion));
      for (int frameInd = 0; frameInd < numFrames; ++frameInd) {
        Animation::Pose pose;
        pose.skeleton = skel;
        pose.jointRotations.resize(numJoints, glm::quat(1.0f, glm::vec3(0.0f)));
        for (int jointInd = 0; jointInd < numJoints; ++jointInd) {
          if (jointInd == 0) {
            // process localPosition only for root joint
            pose.rootLocalPosition =
                animationPerJoint[jointInd][frameInd].localPosition;
          }
          pose.jointRotations[jointInd] =
              animationPerJoint[jointInd][frameInd].localRotation;
        }
        motion->poses.push_back(pose);
      }
    }
  }

  // process the meshes after the bone has setup
  for (auto fbxMesh : scene->meshes) {
    for (auto fbxMeshPart : fbxMesh->material_parts) {
      meshes.push_back(
          ProcessMesh(fbxMesh, fbxMeshPart, boneMapping, modelPath));
    }
  }

  // free the scene object
  ufbx_free_scene(scene);
}

struct TmpVertexFBXLoading {
  glm::vec4 Position;
  glm::vec4 Normal;
  glm::vec4 TexCoords;
  glm::vec4 Color;
  int BoneId[MAX_BONES];
  float BoneWeight[MAX_BONES];
  glm::vec3 BlendShapeOffset[MAX_BLEND_SHAPES];
  glm::vec3 BlendShapeNormal[MAX_BLEND_SHAPES];
};
Render::Mesh *ProcessMesh(ufbx_mesh *mesh, ufbx_mesh_part &part,
                          std::map<string, std::size_t> &boneMapping,
                          std::string modelPath) {
  vector<TmpVertexFBXLoading> vertices;
  vector<unsigned int> indices(mesh->max_face_triangles * 3);
  std::map<string, std::size_t> blendShapeMapping;
  std::vector<BlendShape> blendShapes;
  for (auto faceInd : part.face_indices) {
    ufbx_face face = mesh->faces[faceInd];
    auto numTriangles =
        ufbx_triangulate_face(indices.data(), indices.size(), mesh, face);
    for (auto i = 0; i < numTriangles * 3; ++i) {
      auto index = indices[i];
      TmpVertexFBXLoading v;
      v.Position.x = mesh->vertex_position[index].x;
      v.Position.y = mesh->vertex_position[index].y;
      v.Position.z = mesh->vertex_position[index].z;
      v.Position.w = 1.0f;

      v.Normal.x = mesh->vertex_normal[index].x;
      v.Normal.y = mesh->vertex_normal[index].y;
      v.Normal.z = mesh->vertex_normal[index].z;
      v.Normal.w = 0.0f;

      // for multiple sets of uv, refer to mesh->uv_sets
      // this is by default the first uv set
      if (mesh->uv_sets.count > 0) {
        v.TexCoords.x = mesh->vertex_uv[index].x;
        v.TexCoords.y = mesh->vertex_uv[index].y;
        if (mesh->uv_sets.count > 1) {
          v.TexCoords.z = mesh->uv_sets[1].vertex_uv[index].x;
          v.TexCoords.w = mesh->uv_sets[1].vertex_uv[index].y;
        } else {
          v.TexCoords.z = 0.0f;
          v.TexCoords.w = 0.0f;
        }
      } else
        v.TexCoords = glm::vec4(0.0f);

      if (mesh->vertex_color.exists) {
        v.Color.x = mesh->vertex_color[index].x;
        v.Color.y = mesh->vertex_color[index].y;
        v.Color.z = mesh->vertex_color[index].z;
        v.Color.w = mesh->vertex_color[index].w;
      } else
        v.Color = glm::vec4(1.0f);

      for (int boneCounter = 0; boneCounter < MAX_BONES; ++boneCounter) {
        v.BoneId[boneCounter] = 0;
        v.BoneWeight[boneCounter] = 0.0f;
      }
      // setup skin deformers
      // if there's skin_deformers, the boneMapping won't be empty
      for (auto skin : mesh->skin_deformers) {
        auto vertex = mesh->vertex_indices[index];
        auto skinVertex = skin->vertices[vertex];
        auto numWeights = skinVertex.num_weights;
        if (numWeights > MAX_BONES)
          numWeights = MAX_BONES;
        float totalWeight = 0.0f;
        for (auto k = 0; k < numWeights; ++k) {
          auto skinWeight = skin->weights[skinVertex.weight_begin + k];
          string clusterName =
              skin->clusters[skinWeight.cluster_index]->bone_node->name.data;
          auto boneMapIt = boneMapping.find(clusterName);
          int mappedBoneInd = 0;
          if (boneMapIt == boneMapping.end()) {
            LOG_F(ERROR, "cluster named %s not in boneMapping",
                  clusterName.c_str());
          } else
            mappedBoneInd = boneMapIt->second;
          v.BoneId[k] = mappedBoneInd;
          totalWeight += (float)skinWeight.weight;
          v.BoneWeight[k] = (float)skinWeight.weight;
        }
        // normalize the skin weights
        if (totalWeight != 0.0f) {
          for (auto k = 0; k < numWeights; ++k)
            v.BoneWeight[k] /= totalWeight;
        } else {
          // parent the vertex to root if its not bind
          v.BoneId[0] = 0;
          v.BoneWeight[0] = 1.0f;
        }
      }
      // blend shapes
      for (auto deformer : mesh->blend_deformers) {
        uint32_t vertex = mesh->vertex_indices[index];

        size_t num_blends = deformer->channels.count;
        if (num_blends > MAX_BLEND_SHAPES) {
          num_blends = MAX_BLEND_SHAPES;
        }

        for (size_t i = 0; i < num_blends; i++) {
          ufbx_blend_channel *channel = deformer->channels[i];
          ufbx_blend_shape *shape = channel->target_shape;
          auto it = blendShapeMapping.find(shape->name.data);
          int blendShapeIndex = blendShapeMapping.size();
          if (it == blendShapeMapping.end()) {
            blendShapeMapping[shape->name.data] = blendShapeIndex;
            blendShapes.push_back(BlendShape());
          } else {
            blendShapeIndex = it->second;
          }
          assert(shape); // In theory this could be missing in broken files
          auto &bs = blendShapes[blendShapeIndex];
          bs.name = shape->name.data;
          v.BlendShapeOffset[blendShapeIndex] =
              ConvertToGLM(ufbx_get_blend_shape_vertex_offset(shape, vertex));
        }
      }
      vertices.push_back(v);
    }
  }

  if (vertices.size() != part.num_triangles * 3)
    LOG_F(WARNING, "vertices number inconsistent with part's triangle number");

  ufbx_vertex_stream stream[] = {
      {vertices.data(), vertices.size(), sizeof(TmpVertexFBXLoading)}};
  indices.resize(part.num_triangles * 3);
  auto numVertices = ufbx_generate_indices(stream, 1, indices.data(),
                                           indices.size(), nullptr, nullptr);
  vertices.resize(numVertices);

  // create the final mesh
  std::vector<Vertex> actualVertices(vertices.size());
  for (int i = 0; i < numVertices; i++) {
    actualVertices[i].Position = vertices[i].Position;
    actualVertices[i].Normal = vertices[i].Normal;
    actualVertices[i].TexCoords = vertices[i].TexCoords;
    actualVertices[i].Color = vertices[i].Color;
    for (int j = 0; j < MAX_BONES; j++) {
      actualVertices[i].BoneId[j] = vertices[i].BoneId[j];
      actualVertices[i].BoneWeight[j] = vertices[i].BoneWeight[j];
    }
  }
  auto result = new Render::Mesh(actualVertices, indices);
  result->identifier = mesh->name.data;
  result->modelPath = modelPath;

  if (blendShapeMapping.size() > 0) {
    // update blend shapes
    for (const auto &bsm : blendShapeMapping) {
      blendShapes[bsm.second].data.resize(vertices.size());
      for (int vertId = 0; vertId < vertices.size(); ++vertId) {
        blendShapes[bsm.second].data[vertId].BlendShapeOffset =
            vertices[vertId].BlendShapeOffset[bsm.second];
      }
    }
    result->blendShapes = blendShapes;
  }

  return result;
}

}; // namespace aEngine
