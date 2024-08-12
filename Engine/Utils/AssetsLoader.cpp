#include "Utils/AssetsLoader.hpp"
#include "Utils/Render/MaterialData.hpp"
#include "Utils/Render/Mesh.hpp"
#include "Utils/Render/Shader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include "Scene.hpp"

namespace aEngine {

using std::string;
using std::vector;

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
  for (auto mat : allMaterials) {
    if (mat.second)
      delete mat.second;
  }
}

unsigned int loadAndCreateTextureFromFile(string texturePath);
vector<Render::Mesh *> loadAndCreateMeshFromFile(string modelPath);
Render::Mesh *processMesh(aiMesh *mesh, const aiScene *scene,
                          string &modelPath);
void processNode(aiNode *node, const aiScene *scene,
                 vector<Render::Mesh *> &meshes, string &modelPath);

void AssetsLoader::LoadDefaultAssets() {
  // initialize all the primitives
  // plane
  vector<Vertex> vertices;
  vector<unsigned int> indices;
  vertices.push_back({{0.5f, 0.0f, 0.5f}, {0.0, 1.0, 0.0}, {1.0f, 1.0f}});
  vertices.push_back({{0.5f, 0.0f, -0.5f}, {0.0, 1.0, 0.0}, {1.0f, 0.0f}});
  vertices.push_back({{-0.5f, 0.0f, -0.5f}, {0.0, 1.0, 0.0}, {0.0f, 0.0f}});
  vertices.push_back({{-0.5f, 0.0f, 0.5f}, {0.0, 1.0, 0.0}, {0.0f, 1.0f}});
  indices = {0, 1, 3, 1, 2, 3};
  auto planePrimitive = new Render::Mesh(vertices, indices);
  planePrimitive->identifier = "plane";
  planePrimitive->modelPath = "::planePrimitive";
  allMeshes.insert(std::make_pair("::planePrimitive",
                                  vector<Render::Mesh *>({planePrimitive})));
  // sphere
  auto sphereMesh = loadAndCreateMeshFromFile("./Assets/meshes/sphere.obj");
  allMeshes.insert(std::make_pair("::spherePrimitive", sphereMesh));
  // cube
  auto cubeMesh = loadAndCreateMeshFromFile("./Assets/meshes/cube.obj");
  allMeshes.insert(std::make_pair("::cubePrimitive", cubeMesh));
  // cylinder
  auto cylinderMesh = loadAndCreateMeshFromFile("./Assets/meshes/cylinder.obj");
  allMeshes.insert(std::make_pair("::cylinderPrimitive", cylinderMesh));
  // cone
  auto coneMesh = loadAndCreateMeshFromFile("./Assets/meshes/cone.obj");
  allMeshes.insert(std::make_pair("::conePrimitive", coneMesh));

  // load icons
  Texture *nullIcon = new Texture();
  nullIcon->id = loadAndCreateTextureFromFile("./Assets/icons/NULL.png");
  allTextures.insert(std::make_pair("::NULL_ICON", nullIcon));

  // load default material
  GetMaterial("::base");
}

Animation::Motion *AssetsLoader::GetMotion(std::string motionPath) {
  if (allMotions.find(motionPath) == allMotions.end()) {
    // load new motion
    Animation::Motion *motion = new Animation::Motion();
    std::string extension = fs::path(motionPath).extension().string();
    if (extension == ".bvh") {
      Console.Log("[info]: load motion data from %s\n", motionPath.c_str());
      motion->LoadFromBVH(motionPath);
      allMotions.insert(std::make_pair(motionPath, motion));
      return motion;
    } else {
      Console.Log("[error]: unnsupported motion file extension %s\n", extension.c_str());
      return nullptr;
    }
  } else {
    Console.Log("[info]: get cached motion %s\n", motionPath.c_str());
    return allMotions[motionPath];
  }
}

Texture *AssetsLoader::GetTexture(string texturePath) {
  if (allTextures.find(texturePath) == allTextures.end()) {
    // load new texture
    Texture *newTexture = new Texture();
    newTexture->id = loadAndCreateTextureFromFile(texturePath);
    newTexture->path = texturePath;
    allTextures[texturePath] = newTexture;
    return newTexture;
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
      Console.Log("[error]: the file %s has no mesh\n", modelPath.c_str());
      return result;
    }
    Console.Log("[info]: load model at %s\n", modelPath.c_str());
    allMeshes[modelPath] = modelMeshes;
    return modelMeshes;
  } else {
    auto meshes = allMeshes[modelPath];
    if (modelPath[0] == ':') {
      Console.Log("[info]: load primitive %s\n", modelPath.c_str());
    } else
      Console.Log("[info]: get cached model %s\n", modelPath.c_str());
    return meshes;
  }
}

Render::Mesh *AssetsLoader::GetMesh(string modelPath, string identifier) {
  if (allMeshes.find(modelPath) == allMeshes.end()) {
    // load new model
    auto modelMeshes = loadAndCreateMeshFromFile(modelPath);
    if (modelMeshes.size() == 0) {
      Console.Log("[error]: the file %s has no mesh\n", modelPath.c_str());
      return nullptr;
    }
    for (auto mesh : modelMeshes) {
      if (mesh->identifier == identifier) {
        Console.Log("[info]: load model at %s, get mesh named %s\n",
                    modelPath.c_str(), identifier.c_str());
        return mesh;
      }
    }
    Console.Log("[error]: load model at %s, but no mesh named %s was found\n",
                modelPath.c_str(), identifier.c_str());
    return nullptr;
  } else {
    auto meshes = allMeshes[modelPath];
    if (modelPath[0] == ':') {
      Console.Log("[info]: load primitive %s\n", modelPath.c_str());
      return meshes[0];
    }
    for (auto mesh : meshes) {
      if (mesh->identifier == identifier) {
        Console.Log("[info]: get cached mesh named %s from model %s\n",
                    identifier.c_str(), modelPath.c_str());
        return mesh;
      }
    }
    Console.Log(
        "[error]: model %s has been loaded, but no mesh named %s found\n",
        modelPath.c_str(), identifier.c_str());
    return nullptr;
  }
}

Render::MaterialData *AssetsLoader::GetMaterial(string materialPath) {
  if (allMaterials.find(materialPath) == allMaterials.end()) {
    // create a new material
    Render::MaterialData *newMat = new Render::MaterialData();
    if (materialPath == "::base") {
      newMat->identifier = "base material";
      newMat->path = "::base";
      newMat->SetDiffuseMaterial(); // set to default material
      newMat->LoadShader(); // the default material uses the default shaders
    } else {                // load from file
      std::ifstream input(materialPath);
      if (!input.is_open()) {
        Console.Log("[error]: can't open material data file %s\n",
                    materialPath.c_str());
        return nullptr;
      }
      // deserialize from material file
      Json json;
      input >> json;
      // setup path and identifier for the material
      newMat->path = materialPath;
      newMat->identifier = fs::path(materialPath).stem().string();
      newMat->Deserialize(json);
      input.close();
    }
    allMaterials.insert(std::make_pair(materialPath, newMat));
    return newMat;
  } else
    return allMaterials[materialPath];
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
    Console.Log("[error]: Texture failed to load at path: %s\n",
                texturePath.c_str());
    stbi_image_free(data);
  }

  return textureID;
}

vector<Render::Mesh *> loadAndCreateMeshFromFile(string modelPath) {
  Assimp::Importer importer;
  vector<Render::Mesh *> meshes;
  const aiScene *scene = importer.ReadFile(
      modelPath.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                             aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    Console.Log("[error]: Assimp error: %s\n", importer.GetErrorString());
    return meshes;
  }
  processNode(scene->mRootNode, scene, meshes, modelPath);
  return meshes;
}

Render::Mesh *processMesh(aiMesh *mesh, const aiScene *scene,
                          string &modelPath) {
  vector<Vertex> vertices;
  vector<unsigned int> indices;
  // walk through each of the mesh's vertices
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex;
    glm::vec3 vector;
    // positions
    vector.x = mesh->mVertices[i].x;
    vector.y = mesh->mVertices[i].y;
    vector.z = mesh->mVertices[i].z;
    vertex.Position = vector;
    // normals
    if (mesh->HasNormals()) {
      vector.x = mesh->mNormals[i].x;
      vector.y = mesh->mNormals[i].y;
      vector.z = mesh->mNormals[i].z;
      vertex.Normal = vector;
    }
    // texture coordinates
    if (mesh->mTextureCoords[0]) { // if this model contains texture coordinates
      glm::vec2 vec;
      // a vertex can contain up to 8 different texture coordinates. We thus
      // make the assumption that we won't use models where a vertex can have
      // multiple texture coordinates so we always take the first set (0).
      vec.x = mesh->mTextureCoords[0][i].x;
      vec.y = mesh->mTextureCoords[0][i].y;
      vertex.TexCoords = vec;
    } else
      vertex.TexCoords = glm::vec2(0.0f, 0.0f);

    vertices.push_back(vertex);
  }
  // now wak through each of the mesh's faces (a face is a mesh its triangle)
  // and retrieve the corresponding vertex indices.
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    // retrieve all indices of the face and store them in the indices vector
    for (unsigned int j = 0; j < face.mNumIndices; j++)
      indices.push_back(face.mIndices[j]);
  }

  Render::Mesh *loadedMesh = new Render::Mesh(vertices, indices);
  loadedMesh->identifier = string(mesh->mName.C_Str());
  loadedMesh->modelPath = modelPath;

  return loadedMesh;
}

void processNode(aiNode *node, const aiScene *scene,
                 vector<Render::Mesh *> &meshes, string &modelPath) {
  // process each mesh located at the current node
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    // the node object only contains indices to index the actual objects in
    // the scene. the scene contains all the data, node is just to keep stuff
    // organized (like relations between nodes).
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(processMesh(mesh, scene, modelPath));
  }
  // after we've processed all of the meshes (if any) we then recursively
  // process each of the children nodes
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene, meshes, modelPath);
  }
}

}; // namespace aEngine