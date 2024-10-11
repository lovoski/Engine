#include "Component/Animator.hpp"
#include "Component/DeformRenderer.hpp"
#include "Component/MeshRenderer.hpp"
#include "Component/NativeScript.hpp"

#include "Function/AssetsLoader.hpp"
#include "Function/Math/Math.hpp"
#include "Function/Render/Mesh.hpp"
#include "Function/Render/Passes/Header.hpp"
#include "Function/Render/RenderPass.hpp"
#include "Function/Render/Shader.hpp"
#include "Function/ShaderCode.hpp"


#include "System/Animation/AnimationSystem.hpp"

#include <tinyobjloader.h>
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

AssetsLoader::AssetsLoader() {}

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
  for (auto motion : allMotions) {
    if (motion.second)
      delete motion.second;
  }
  allSkeletons.clear();
}

unsigned int loadAndCreateTextureFromFile(string texturePath,
                                          bool flipVertically = true);
unsigned int loadHDRImageFromFile(string filePath, bool flipVertically = true);

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
  auto sphereMesh =
      loadAndCreateAssetsFromFile(ASSETS_PATH "/meshes/sphere.fbx");
  sphereMesh[0]->identifier = "sphere";
  sphereMesh[0]->modelPath = "::spherePrimitive";
  allMeshes.insert(std::make_pair("::spherePrimitive", sphereMesh));
  // cube
  auto cubeMesh = loadAndCreateAssetsFromFile(ASSETS_PATH "/meshes/cube.fbx");
  cubeMesh[0]->identifier = "cube";
  cubeMesh[0]->modelPath = "::cubePrimitive";
  allMeshes.insert(std::make_pair("::cubePrimitive", cubeMesh));
  // cylinder
  auto cylinderMesh =
      loadAndCreateAssetsFromFile(ASSETS_PATH "/meshes/cylinder.fbx");
  cylinderMesh[0]->identifier = "cylinder";
  cylinderMesh[0]->modelPath = "::cylinderPrimitive";
  allMeshes.insert(std::make_pair("::cylinderPrimitive", cylinderMesh));

  // load default textures
  Texture *nullTexture = new Texture();
  nullTexture->id =
      loadAndCreateTextureFromFile(ASSETS_PATH "/textures/null.png");
  nullTexture->path = "::null_texture";
  allTextures.insert(std::make_pair("::null_texture", nullTexture));

  Texture *whiteTexture = new Texture();
  whiteTexture->id =
      loadAndCreateTextureFromFile(ASSETS_PATH "/textures/white.png");
  whiteTexture->path = "::white_texture";
  allTextures.insert(std::make_pair("::white_texture", whiteTexture));

  Texture *blackTexture = new Texture();
  blackTexture->id =
      loadAndCreateTextureFromFile(ASSETS_PATH "/textures/black.png");
  blackTexture->path = "::black_texture";
  allTextures.insert(std::make_pair("::black_texture", blackTexture));

  // load shaders
  prepareDefaultShader(Render::basicVS, Render::basicFS, Render::basicGS,
                       "::basic");
  prepareDefaultShader(errorVS, errorFS, "none", "::error");
  prepareDefaultShader(Render::outlineVS, Render::outlineFS, "none",
                       "::outline");
  prepareDefaultShader(Render::GBVMainVS, Render::GBVMainFS, "none",
                       "::gbvmain");
  prepareDefaultShader(shadowMapDirLightVS, shadowMapDirLightFS, "none",
                       "::shadowMapDirLight");
  prepareDefaultShader(Render::wireframeVS, Render::wireframeFS, "none",
                       "::wireframe");
  prepareDefaultShader(Render::pbrVS, Render::pbrFS, "none", "::pbr");
  prepareDefaultShader(Render::fvpVS, Render::fvpFS, Render::fvpGS, "::fvp");
}

void AssetsLoader::prepareDefaultShader(std::string vs, std::string fs,
                                        std::string gs,
                                        std::string identifier) {
  auto shader = new Render::Shader();
  shader->identifier = identifier;
  shader->LoadAndRecompileShaderSource(vs, fs, gs);
  allShaders.insert(std::make_pair(identifier, shader));
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
std::shared_ptr<Render::Shader>
AssetsLoader::GetShader(std::string identifier) {
  auto s = allShaders.find(identifier);
  if (s == allShaders.end()) {
    LOG_F(ERROR, "shader with identifier %s not found", identifier.c_str());
    return GetShader("::error");
  } else {
    LOG_F(INFO, "get shader with identifier %s", identifier.c_str());
    return (*s).second;
  }
}
std::shared_ptr<Render::Shader>
AssetsLoader::GetShader(std::string vsp, std::string fsp, std::string gsp) {
  std::shared_ptr<Render::Shader> newShader =
      std::make_shared<Render::Shader>();
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

Animation::Skeleton *AssetsLoader::GetActor(std::string filepath) {
  if (allSkeletons.find(filepath) == allSkeletons.end())
    auto motion = GetMotion(filepath);
  if (allSkeletons.find(filepath) == allSkeletons.end()) {
    LOG_F(ERROR, "can't find actor from file %s", filepath.c_str());
    return nullptr;
  } else {
    LOG_F(INFO, "load actor from %s", filepath.c_str());
    return allSkeletons[filepath];
  }
}

Animation::Motion *AssetsLoader::GetMotion(std::string motionPath) {
  if (allMotions.find(motionPath) == allMotions.end()) {
    // load new motion
    Animation::Motion *motion = new Animation::Motion();
    std::string extension = fs::path(motionPath).extension().string();
    if (extension == ".bvh") {
      LOG_F(INFO, "load motion data from %s", motionPath.c_str());
      motion->LoadFromBVH(motionPath);
      motion->path = motionPath;
      motion->skeleton.path = motionPath;
      allMotions.insert(std::make_pair(motionPath, motion));
      allSkeletons.insert(std::make_pair(motionPath, &motion->skeleton));
      return motion;
    } else if (extension == ".fbx") {
      LOG_F(INFO, "load motion data from %s", motionPath.c_str());
      auto meshes = loadAndCreateAssetsFromFile(motionPath);
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

Texture *AssetsLoader::GetTexture(string texturePath, bool flipVertically) {
  if (allTextures.find(texturePath) == allTextures.end()) {
    // load new texture
    Texture *newTexture = new Texture();
    auto id = loadAndCreateTextureFromFile(texturePath, flipVertically);
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

Texture *AssetsLoader::GetHDRTexture(std::string texturePath,
                                     bool flipVertically) {
  if (allTextures.find(texturePath) == allTextures.end()) {
    // load new texture
    Texture *newTexture = new Texture();
    auto id = loadHDRImageFromFile(texturePath, flipVertically);
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
    auto modelMeshes = loadAndCreateAssetsFromFile(modelPath);
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
    auto modelMeshes = loadAndCreateAssetsFromFile(modelPath);
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

unsigned int loadHDRImageFromFile(string filePath, bool flipVertically) {
  stbi_set_flip_vertically_on_load(flipVertically);
  int width, height, nrComponents;
  float *data = stbi_loadf(filePath.c_str(), &width, &height, &nrComponents, 0);
  unsigned int hdrTexture;
  if (data) {
    glGenTextures(1, &hdrTexture);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB,
                 GL_FLOAT, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    LOG_F(ERROR, "failed to hdr image from %s", filePath.c_str());
    hdrTexture = (unsigned int)(-1);
  }
  return hdrTexture;
}

unsigned int loadAndCreateTextureFromFile(string texturePath,
                                          bool flipVertically) {
  unsigned int textureID;
  glGenTextures(1, &textureID);

  stbi_set_flip_vertically_on_load(flipVertically);
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
    meshes = loadAndCreateAssetsFromFile(modelPath);
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
  }

  auto meshParent = GWORLD.AddNewEntity();
  globalParent->AssignChild(meshParent.get());
  meshParent->name = "mesh";
  auto globalMaterial =
      Loader.InstantiateMaterial<Render::Basic>(globalParent->name);
  for (auto mesh : meshes) {
    auto c = GWORLD.AddNewEntity();
    c->name = mesh->identifier;
    if (skel != nullptr) {
      // add deform renderer
      c->AddComponent<Mesh>(mesh);
      c->AddComponent<DeformRenderer>(globalParent->ID);
      c->GetComponent<DeformRenderer>()->renderer->AddPass(
          globalMaterial, globalMaterial->identifier);
      meshParent->AssignChild(c.get());
    } else {
      c->AddComponent<Mesh>(mesh);
      c->AddComponent<MeshRenderer>();
      c->GetComponent<MeshRenderer>()->AddPass(globalMaterial,
                                               globalMaterial->identifier);
      meshParent->AssignChild(c.get());
    }
  }

  return globalParent;
}

Render::Mesh *ProcessMesh(ufbx_mesh *mesh, ufbx_mesh_part &part,
                          std::map<string, std::size_t> &boneMapping,
                          std::string modelPath) {
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
  result->modelPath = modelPath;
  return result;
}

void AssetsLoader::loadOBJModelFile(std::vector<Render::Mesh *> &meshes,
                                    std::string modelPath) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err, warn;
  if (tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                       modelPath.c_str())) {
    for (auto &shape : shapes) {
      std::size_t indexOffset = 0;

      std::vector<Vertex> vertices;
      std::vector<unsigned int> indices;

      for (auto f = 0; f < shape.mesh.num_face_vertices.size(); ++f) {
        auto fv = shape.mesh.num_face_vertices[f];
        if (fv != 3) {
          LOG_F(WARNING,
                "only triangle faces are handled, face id=%d has %d vertices",
                f, fv);
          indexOffset += fv;
          continue;
        }
        Vertex vert[3];

        bool withoutNormal = false;
        for (auto v = 0; v < 3; ++v) {
          auto idx = shape.mesh.indices[indexOffset + v];
          vert[v].Position =
              glm::vec4(attrib.vertices[3 * idx.vertex_index + 0],
                        attrib.vertices[3 * idx.vertex_index + 1],
                        attrib.vertices[3 * idx.vertex_index + 2], 1.0f);
          if (idx.normal_index >= 0) {
            vert[v].Normal =
                glm::vec4(attrib.normals[3 * idx.normal_index + 0],
                          attrib.normals[3 * idx.normal_index + 1],
                          attrib.normals[3 * idx.normal_index + 2], 0.0f);
          } else {
            withoutNormal = true;
            vert[v].Normal = glm::vec4(0.0f);
          }
          if (idx.texcoord_index >= 0) {
            vert[v].TexCoords = glm::vec4(
                attrib.texcoords[2 * idx.texcoord_index + 0],
                attrib.texcoords[2 * idx.texcoord_index + 1], 1.0f, 1.0f);
          } else
            vert[v].TexCoords = glm::vec4(0.0f);
        }
        if (withoutNormal) {
          // manually compute the normal
          auto faceNormal = Math::FaceNormal(vert[0].Position, vert[1].Position,
                                             vert[2].Position);
          vert[0].Normal = glm::vec4(faceNormal, 0.0f);
          vert[1].Normal = glm::vec4(faceNormal, 0.0f);
          vert[2].Normal = glm::vec4(faceNormal, 0.0f);
        }

        vertices.push_back(vert[0]);
        vertices.push_back(vert[1]);
        vertices.push_back(vert[2]);
        indices.push_back(indexOffset + 0);
        indices.push_back(indexOffset + 1);
        indices.push_back(indexOffset + 2);

        indexOffset += fv;
      }

      Render::Mesh *mesh = new Render::Mesh(vertices, indices);
      mesh->identifier = shape.name;
      mesh->modelPath = modelPath;
      meshes.push_back(mesh);
    }
  } else {
    LOG_F(ERROR, "failed to load .obj model from %s", modelPath.c_str());
  }
}

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

vector<Render::Mesh *>
AssetsLoader::loadAndCreateAssetsFromFile(string modelPath) {
  vector<Render::Mesh *> meshes;

  auto extension = fs::path(modelPath).extension().string();
  if (extension == ".fbx") {
    // load fbx model with ufbx
    loadFBXModelFile(meshes, modelPath);
  } else if (extension == ".obj") {
    // load obj model with custom loader
    loadOBJModelFile(meshes, modelPath);
  }

  return meshes;
}

}; // namespace aEngine