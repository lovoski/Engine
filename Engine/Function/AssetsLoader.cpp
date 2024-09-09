#include "Scripts/Animation/SAMERetarget.hpp"
#include "Scripts/Animation/VisMetrics.hpp"

#include "Component/Animator.hpp"
#include "Component/DeformRenderer.hpp"
#include "Component/MeshRenderer.hpp"
#include "Component/NativeScript.hpp"

#include "Function/AssetsLoader.hpp"
#include "Function/HardCodeAssets.hpp"
#include "Function/Render/Mesh.hpp"
#include "Function/Render/RenderPass.hpp"
#include "Function/Render/Shader.hpp"

#include "System/Animation/AnimationSystem.hpp"

#include <ufbx.h>

#include "Scene.hpp"

namespace aEngine {

using std::string;
using std::vector;

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

AssetsLoader::AssetsLoader() {
  // stb image library setup
  stbi_set_flip_vertically_on_load(true);
}

AssetsLoader::~AssetsLoader() {
  for (auto texture : allTextures) {
    if (texture.second) {
      glDeleteTextures(1, &texture.second->id);
      delete texture.second;
    }
  }
  for (auto meshEle : allMeshes) {
    for (auto mesh : meshEle.second)
      if (mesh)
        delete mesh;
  }
  for (auto shader : allShaders) {
    if (shader.second)
      delete shader.second;
  }
  for (auto skeleton : allSkeletons) {
    if (skeleton.second)
      delete skeleton.second;
  }
  allSkeletons.clear();
}

unsigned int loadAndCreateTextureFromFile(string texturePath);

void AssetsLoader::LoadDefaultAssets() {
  // initialize all the primitives
  // plane
  vector<Vertex> vertices;
  vector<unsigned int> indices;
  vertices.push_back({{0.5f, 0.0f, 0.5f, 1.0f},
                      {0.0, 1.0, 0.0, 0.0f},
                      {1.0f, 1.0f, 1.0f, 1.0f}});
  vertices.push_back({{0.5f, 0.0f, -0.5f, 1.0f},
                      {0.0, 1.0, 0.0, 0.0f},
                      {1.0f, 0.0f, 1.0f, 1.0f}});
  vertices.push_back({{-0.5f, 0.0f, -0.5f, 1.0f},
                      {0.0, 1.0, 0.0, 0.0f},
                      {0.0f, 0.0f, 1.0f, 1.0f}});
  vertices.push_back({{-0.5f, 0.0f, 0.5f, 1.0f},
                      {0.0, 1.0, 0.0, 0.0f},
                      {0.0f, 1.0f, 1.0f, 1.0f}});
  indices = {0, 1, 3, 1, 2, 3};
  auto planePrimitive = new Render::Mesh(vertices, indices);
  planePrimitive->identifier = "plane";
  planePrimitive->modelPath = "::planePrimitive";
  allMeshes.insert(std::make_pair("::planePrimitive",
                                  vector<Render::Mesh *>({planePrimitive})));
  // sphere
  auto sphereMesh = loadAndCreateMeshFromFile("./Assets/meshes/sphere.fbx");
  allMeshes.insert(std::make_pair("::spherePrimitive", sphereMesh));
  // cube
  auto cubeMesh = loadAndCreateMeshFromFile("./Assets/meshes/cube.fbx");
  allMeshes.insert(std::make_pair("::cubePrimitive", cubeMesh));
  // cylinder
  auto cylinderMesh = loadAndCreateMeshFromFile("./Assets/meshes/cylinder.fbx");
  allMeshes.insert(std::make_pair("::cylinderPrimitive", cylinderMesh));

  // load default textures
  Texture *nullTexture = new Texture();
  nullTexture->id = loadAndCreateTextureFromFile("./Assets/textures/null.png");
  nullTexture->path = "::null_texture";
  allTextures.insert(std::make_pair("::null_texture", nullTexture));

  Texture *whiteTexture = new Texture();
  whiteTexture->id =
      loadAndCreateTextureFromFile("./Assets/textures/white.png");
  whiteTexture->path = "::white_texture";
  allTextures.insert(std::make_pair("::white_texture", whiteTexture));

  // load shaders
  Render::Shader *diffuseShader = new Render::Shader();
  diffuseShader->identifier = "::diffuse";
  diffuseShader->LoadAndRecompileShaderSource(diffuseVS, diffuseFS);
  allShaders.insert(std::make_pair("::diffuse", diffuseShader));

  Render::Shader *errorShader = new Render::Shader();
  errorShader->identifier = "::error";
  errorShader->LoadAndRecompileShaderSource(errorVS, errorFS);
  allShaders.insert(std::make_pair("::error", errorShader));

  Render::Shader *outlineShader = new Render::Shader();
  outlineShader->identifier = "::outline";
  outlineShader->LoadAndRecompileShaderSource(outlineVS, outlineFS);
  allShaders.insert(std::make_pair("::outline", outlineShader));

  Render::Shader *gbvMainShader = new Render::Shader();
  gbvMainShader->identifier = "::gbvmain";
  gbvMainShader->LoadAndRecompileShaderSource(GBVMainVS, GBVMainFS);
  allShaders.insert(std::make_pair("::gbvmain", gbvMainShader));

  Render::Shader *shadowMapDirLight = new Render::Shader();
  shadowMapDirLight->identifier = "::shadowMapDirLight";
  shadowMapDirLight->LoadAndRecompileShaderSource(shadowMapDirLightVS,
                                                  shadowMapDirLightFS);
  allShaders.insert(std::make_pair("::shadowMapDirLight", shadowMapDirLight));
}

std::vector<std::string> AssetsLoader::GetIndetifiersForAllCachedMaterials() {
  std::vector<std::string> result;
  for (auto m : allMaterials)
    result.push_back(m->identifier);
  return result;
}

std::vector<std::string> AssetsLoader::GetIdentifiersForAllCachedShaders() {
  std::vector<std::string> result;
  for (auto s : allShaders) {
    result.push_back(s.first);
  }
  return result;
}
Render::Shader *AssetsLoader::GetShader(std::string identifier) {
  auto s = allShaders.find(identifier);
  if (s == allShaders.end()) {
    LOG_F(ERROR, "shader with identifier %s not found", identifier.c_str());
    return GetShader("::error");
  } else {
    LOG_F(INFO, "get shader with identifier %s", identifier.c_str());
    return (*s).second;
  }
}
Render::Shader *AssetsLoader::GetShader(std::string vsp, std::string fsp,
                                        std::string gsp) {
  Render::Shader *newShader = new Render::Shader();
  if (newShader->LoadAndRecompileShader(vsp, fsp, gsp)) {
    // create identifier for this shader
    newShader->identifier = fs::path(vsp).stem().string() + ":" +
                            fs::path(fsp).stem().string() + ":" +
                            fs::path(gsp).stem().string();
    allShaders.insert(std::make_pair(newShader->identifier, newShader));
    LOG_F(INFO,
          "load shader from path, identifier as %s vsp=%s, fsp=%s, gsp=%s",
          newShader->identifier.c_str(), vsp.c_str(), fsp.c_str(), gsp.c_str());
    return newShader;
  } else
    return GetShader(":error");
}

Animation::Motion *AssetsLoader::GetMotion(std::string motionPath) {
  if (allMotions.find(motionPath) == allMotions.end()) {
    // load new motion
    Animation::Motion *motion = new Animation::Motion();
    std::string extension = fs::path(motionPath).extension().string();
    if (extension == ".bvh") {
      LOG_F(INFO, "load motion data from %s", motionPath.c_str());
      motion->LoadFromBVH(motionPath);
      allMotions.insert(std::make_pair(motionPath, motion));
      return motion;
    } else if (extension == ".fbx") {
      LOG_F(INFO, "load motion data from %s", motionPath.c_str());
      auto meshes = loadAndCreateMeshFromFile(motionPath);
      // keep record of the loaded mesh
      allMeshes.insert(std::make_pair(motionPath, meshes));
      auto it = allMotions.find(motionPath);
      if (it == allMotions.end())
        return nullptr;
      else
        return it->second;
    } else {
      LOG_F(ERROR, "GetMotion only loads bvh and fbx motion");
      return nullptr;
    }
  } else {
    LOG_F(INFO, "get cached motion %s", motionPath.c_str());
    return allMotions[motionPath];
  }
}

Texture *AssetsLoader::GetTexture(string texturePath) {
  if (allTextures.find(texturePath) == allTextures.end()) {
    // load new texture
    Texture *newTexture = new Texture();
    auto id = loadAndCreateTextureFromFile(texturePath);
    if (id == (unsigned int)(-1)) {
      // failed to load texture, return null
      return allTextures["::null_texture"];
    } else {
      newTexture->id = id;
      newTexture->path = texturePath;
      allTextures[texturePath] = newTexture;
      return newTexture;
    }
  } else {
    return allTextures[texturePath];
  }
}

std::vector<Render::Mesh *> AssetsLoader::GetModel(std::string modelPath) {
  std::vector<Render::Mesh *> result;
  if (allMeshes.find(modelPath) == allMeshes.end()) {
    // load new model
    auto modelMeshes = loadAndCreateMeshFromFile(modelPath);
    if (modelMeshes.size() == 0) {
      LOG_F(ERROR, "the file %s has no mesh", modelPath.c_str());
      return result;
    }
    LOG_F(INFO, "load model at %s", modelPath.c_str());
    allMeshes[modelPath] = modelMeshes;
    return modelMeshes;
  } else {
    auto meshes = allMeshes[modelPath];
    if (modelPath[0] == ':') {
      LOG_F(INFO, "load primitive %s", modelPath.c_str());
    } else
      LOG_F(INFO, "get cached model %s", modelPath.c_str());
    return meshes;
  }
}

Render::Mesh *AssetsLoader::GetMesh(string modelPath, string identifier) {
  if (allMeshes.find(modelPath) == allMeshes.end()) {
    // load new model
    auto modelMeshes = loadAndCreateMeshFromFile(modelPath);
    if (modelMeshes.size() == 0) {
      LOG_F(ERROR, "the file %s has no mesh", modelPath.c_str());
      return nullptr;
    }
    for (auto mesh : modelMeshes) {
      if (mesh->identifier == identifier) {
        LOG_F(INFO, "load model at %s, get mesh named %s", modelPath.c_str(),
              identifier.c_str());
        return mesh;
      }
    }
    LOG_F(ERROR, "load model at %s, but no mesh named %s was found",
          modelPath.c_str(), identifier.c_str());
    return nullptr;
  } else {
    auto meshes = allMeshes[modelPath];
    if (modelPath[0] == ':') {
      LOG_F(INFO, "load primitive %s", modelPath.c_str());
      return meshes[0];
    }
    for (auto mesh : meshes) {
      if (mesh->identifier == identifier) {
        LOG_F(INFO, "get cached mesh named %s from model %s",
              identifier.c_str(), modelPath.c_str());
        return mesh;
      }
    }
    LOG_F(ERROR, "model %s has been loaded, but no mesh named %s found",
          modelPath.c_str(), identifier.c_str());
    return nullptr;
  }
}

unsigned int loadAndCreateTextureFromFile(string texturePath) {
  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  unsigned char *data =
      stbi_load(texturePath.c_str(), &width, &height, &nrComponents, 0);
  if (data) {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    LOG_F(ERROR, "Texture failed to load at path: %s", texturePath.c_str());
    // set the id of texture to (unsigned int)(-1) if failed to load
    textureID = (unsigned int)(-1);
    stbi_image_free(data);
  }

  return textureID;
}

std::shared_ptr<Entity>
AssetsLoader::LoadAndCreateEntityFromFile(string modelPath) {
  auto globalParent = GWORLD.AddNewEntity();
  globalParent->name = fs::path(modelPath).filename().stem().string();

  Animation::Skeleton *skel = nullptr;
  Animation::Motion *motion = nullptr;
  vector<Render::Mesh *> meshes;
  if (allMeshes.find(modelPath) == allMeshes.end()) {
    // new asset
    meshes = loadAndCreateMeshFromFile(modelPath);
    allMeshes.insert(std::make_pair(modelPath, meshes));
  } else {
    meshes = allMeshes[modelPath];
  }
  auto it = allSkeletons.find(modelPath);
  if (it != allSkeletons.end()) {
    // this model contains skeleton
    skel = (*it).second;
  }
  auto motionIt = allMotions.find(modelPath);
  if (motionIt != allMotions.end()) {
    motion = (*motionIt).second;
  }

  if (skel != nullptr) {
    globalParent->AddComponent<Animator>(skel);
    globalParent->AddComponent<NativeScript>();
    globalParent->GetComponent<NativeScript>()->Bind<SAMERetarget>();
    globalParent->GetComponent<NativeScript>()->Bind<VisMetrics>();
  }

  auto meshParent = GWORLD.AddNewEntity();
  globalParent->AssignChild(meshParent.get());
  meshParent->name = "mesh";
  auto globalMaterial =
      Loader.InstantiateMaterial<Render::Diffuse>(globalParent->name);
  for (auto mesh : meshes) {
    auto c = GWORLD.AddNewEntity();
    c->name = mesh->identifier;
    if (skel != nullptr) {
      // add deform renderer
      c->AddComponent<DeformRenderer>(
          mesh, globalParent->GetComponent<Animator>().get());
      c->GetComponent<DeformRenderer>()->AddPass(globalMaterial,
                                                 globalMaterial->identifier);
      meshParent->AssignChild(c.get());
    } else {
      c->AddComponent<MeshRenderer>(mesh);
      c->GetComponent<MeshRenderer>()->AddPass(globalMaterial,
                                               globalMaterial->identifier);
      meshParent->AssignChild(c.get());
    }
  }

  return globalParent;
}

Render::Mesh *ProcessMesh(ufbx_mesh *mesh, ufbx_mesh_part &part,
                          std::map<string, std::size_t> &boneMapping) {
  vector<Vertex> vertices;
  vector<unsigned int> indices(mesh->max_face_triangles * 3);
  for (auto faceInd : part.face_indices) {
    ufbx_face face = mesh->faces[faceInd];
    auto numTriangles =
        ufbx_triangulate_face(indices.data(), indices.size(), mesh, face);
    for (auto i = 0; i < numTriangles * 3; ++i) {
      auto index = indices[i];
      Vertex v;
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
      v.TexCoords.x = mesh->vertex_uv[index].x;
      v.TexCoords.y = mesh->vertex_uv[index].y;
      v.TexCoords.z = 1.0f;
      v.TexCoords.w = 1.0f;
      if (mesh->uv_sets.count > 1) {
        // setup the second set of uv
        v.TexCoords.z = mesh->uv_sets[1].vertex_uv[index].x;
        v.TexCoords.w = mesh->uv_sets[1].vertex_uv[index].y;
      }

      v.Color = glm::vec4(1.0f);
      if (mesh->vertex_color.exists) {
        v.Color.x = mesh->vertex_color[index].x;
        v.Color.y = mesh->vertex_color[index].y;
        v.Color.z = mesh->vertex_color[index].z;
        v.Color.w = mesh->vertex_color[index].w;
      }

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

      vertices.push_back(v);
    }
  }

  if (vertices.size() != part.num_triangles * 3)
    LOG_F(WARNING, "vertices number inconsistent with part's triangle number");

  ufbx_vertex_stream stream[] = {
      {vertices.data(), vertices.size(), sizeof(Vertex)}};
  indices.resize(part.num_triangles * 3);
  auto numVertices = ufbx_generate_indices(stream, 1, indices.data(),
                                           indices.size(), nullptr, nullptr);
  vertices.resize(numVertices);

  // create the final mesh
  auto result = new Render::Mesh(vertices, indices);
  result->identifier = mesh->name.data;
  return result;
}

vector<Render::Mesh *>
AssetsLoader::loadAndCreateMeshFromFile(string modelPath) {
  vector<Render::Mesh *> meshes;

  ufbx_load_opts opts = {};
  opts.target_axes = ufbx_axes_right_handed_y_up;
  opts.target_unit_meters = 1.0f;
  ufbx_error error;
  ufbx_scene *scene = ufbx_load_file(modelPath.c_str(), &opts, &error);
  if (!scene) {
    LOG_F(ERROR, "failed to load scene: %s", error.description.data);
    return meshes;
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
      meshes.push_back(ProcessMesh(fbxMesh, fbxMeshPart, boneMapping));
    }
  }

  // free the scene object
  ufbx_free_scene(scene);

  return meshes;
}

}; // namespace aEngine