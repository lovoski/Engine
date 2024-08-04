#include "ResourceManager.hpp"
#include "MaterialData.hpp"
#include "ecs/components/Material.hpp"
#include "ecs/components/MeshRenderer.hpp"
#include "geometry/Mesh.hpp"

#include <filesystem>

namespace fs = std::filesystem;

namespace Resource {

ResourceManager::ResourceManager() {}

ResourceManager::~ResourceManager() {
  for (auto texture : allTextures) {
    if (texture.second) {
      glDeleteTextures(1, &texture.second->id);
      delete texture.second;
    }
  }
  for (auto shader : allShaders) {
    if (shader) {
      delete shader;
    }
  }
  for (auto meshEle : allMeshes) {
    for (auto mesh : meshEle.second)
      if (mesh)
        delete mesh;
  }
  for (auto icon : allIcons)
    if (icon.second) {
      glDeleteTextures(1, &icon.second->id);
      delete icon.second;
    }
  for (auto mat : allMaterials) {
    if (mat.second)
      delete mat.second;
  }
  if (planePrimitive)
    delete planePrimitive;
  if (spherePrimitive)
    delete spherePrimitive;
  if (cubePrimitive)
    delete cubePrimitive;
  if (cylinderPrimitive)
    delete cylinderPrimitive;
  if (conePrimitive)
    delete conePrimitive;
  if (TextureSlot)
    delete TextureSlot;
}

void ResourceManager::Initialize() {

  // stb image library setup
  stbi_set_flip_vertically_on_load(true);

  // initialize all the primitives
  // plane
  vector<Vertex> vertices;
  vector<unsigned int> indices;
  vertices.push_back({{0.5f, 0.0f, 0.5f}, {0.0, 1.0, 0.0}, {1.0f, 1.0f}});
  vertices.push_back({{0.5f, 0.0f, -0.5f}, {0.0, 1.0, 0.0}, {1.0f, 0.0f}});
  vertices.push_back({{-0.5f, 0.0f, -0.5f}, {0.0, 1.0, 0.0}, {0.0f, 0.0f}});
  vertices.push_back({{-0.5f, 0.0f, 0.5f}, {0.0, 1.0, 0.0}, {0.0f, 1.0f}});
  indices = {0, 1, 3, 1, 2, 3};
  planePrimitive = new Graphics::Mesh(vertices, indices);
  planePrimitive->identifier = "plane";
  planePrimitive->modelPath = "::planePrimitive";
  // sphere
  auto sphereMesh =
      getModel("./default/primitives/sphere.obj");
  spherePrimitive =
      new Graphics::Mesh(sphereMesh[0]->vertices, sphereMesh[0]->indices);
  spherePrimitive->identifier = "sphere";
  spherePrimitive->modelPath = "::spherePrimitive";
  // cube
  auto cubeMesh = getModel("./default/primitives/cube.obj");
  cubePrimitive =
      new Graphics::Mesh(cubeMesh[0]->vertices, cubeMesh[0]->indices);
  cubePrimitive->identifier = "cube";
  cubePrimitive->modelPath = "::cubePrimitive";
  // cylinder
  auto cylinderMesh =
      getModel("./default/primitives/cylinder.obj");
  cylinderPrimitive =
      new Graphics::Mesh(cylinderMesh[0]->vertices, cylinderMesh[0]->indices);
  cylinderPrimitive->identifier = "cylinder";
  cylinderPrimitive->modelPath = "::cylinderPrimitive";
  // cone
  auto coneMesh = getModel("./default/primitives/cone.obj");
  conePrimitive =
      new Graphics::Mesh(coneMesh[0]->vertices, coneMesh[0]->indices);
  conePrimitive->identifier = "cone";
  conePrimitive->modelPath = "::conePrimitive";

  // load icons
  Texture *nullIcon = new Texture();
  nullIcon->id = textureFromFile("./default/icons/NULL.png");
  allIcons.insert(std::make_pair(ICON_TYPE::NULL_TYPE, nullIcon));
  // initialize texture slot
  TextureSlot = new Texture();
  // copy the null icon's id
  TextureSlot->id = nullIcon->id;
  TextureSlot->path = "::textureSlot";
  // load default shaders
  Shader *baseShader =
      new Shader("./default/shaders/base.vert",
                 "./default/shaders/base.frag");
  baseShader->identifier = "base shader";
  allShaders.push_back(baseShader);
  Shader *errorShader =
      new Shader("./default/shaders/error.vert",
                 "./default/shaders/error.frag");
  errorShader->identifier = "base shader";
  allShaders.push_back(errorShader);
  // load default material
  GetMaterialData("::base");
}

MaterialData *ResourceManager::GetMaterialData(string path) {
  if (allMaterials.find(path) == allMaterials.end()) {
    // create a new material
    MaterialData *newMat = new MaterialData();
    if (path == "::base") {
      // the first time call GetMaterialData("::base")
      newMat->identifier = "base material";
      newMat->path = "::base";
      newMat->SetDefaultMaterial(); // set to default material
    } else {                        // load from file
      std::ifstream input(path);
      if (!input.is_open()) {
        cout << "can't open material data file " << path << endl;
        return nullptr;
      }
      // deserialize from material file
      Json json;
      input >> json;
      newMat->path = path;
      newMat->identifier = fs::path(path).stem().string();
      newMat->Deserialize(json);
      vector<int> texIndices = json["texVariables"]["indices"];
      vector<string> texPathes = json["texVariables"]["pathes"];
      for (int i = 0; i < texIndices.size(); ++i) {
        if (texPathes[i] == "::textureSlot") {
          // if this is a empty slot
          // this pointer to empty slot will be replaced by the pointer to a actual
          // texture after user upload the texture
          newMat->texVariables.insert(std::make_pair(texIndices[i], TextureSlot));
        } else {
          // if this texture has some actual data
          newMat->texVariables.insert(
              std::make_pair(texIndices[i], GetTexture(texPathes[i])));
        }
      }
      input.close();
    }
    allMaterials.insert(std::make_pair(path, newMat));
    return newMat;
  } else
    return allMaterials[path];
}

Texture *ResourceManager::GetTexture(string path, bool forceReload) {
  if (allTextures.find(path) == allTextures.end()) {
    Texture *newTex = new Texture();
    newTex->id = textureFromFile(path);
    newTex->path = path;
    allTextures.insert(std::make_pair(path, newTex));
    return newTex;
  } else {
    auto ptr = allTextures[path];
    if (forceReload) {
      glDeleteTextures(1, &ptr->id);
      ptr->id = textureFromFile(path);
    }
    return ptr;
  }
}

vector<Mesh *> ResourceManager::GetModel(string path) {
  if (allMeshes.find(path) == allMeshes.end()) {
    auto meshes = getModel(path);
    allMeshes.insert(std::make_pair(path, meshes));
    return meshes;
  } else {
    return allMeshes[path];
  }
}

Mesh *ResourceManager::GetMesh(string path, string identifier) {
  if (path == "::cubePrimitive") return cubePrimitive;
  else if (path == "::spherePrimitive") return spherePrimitive;
  else if (path == "::planePrimitive") return planePrimitive;
  else if (path == "::cylinderPrimitive") return cylinderPrimitive;
  else if (path == "::conePrimitive") return conePrimitive;
  else;
  if (allMeshes.find(path) == allMeshes.end()) {
    // the model has not been loaded
    auto meshes = getModel(path);
    allMeshes.insert(std::make_pair(path, meshes));
    for (auto mesh : meshes) {
      if (identifier == mesh->identifier) {
        return mesh;
      }
    }
    printf(
        "the model has now been loaded, but no mesh named %s has been found\n",
        identifier.c_str());
    return nullptr;
  } else {
    auto meshes = allMeshes[path];
    for (auto mesh : meshes) {
      if (identifier == mesh->identifier) {
        return mesh;
      }
    }
    printf("the model was loaded before, but no mesh named %s has been found\n",
           identifier.c_str());
    return nullptr;
  }
}

Shader *ResourceManager::GetShader(string vertShaderPath, string fragShaderPath,
                                   string geomShaderPath, bool forceReload) {
  for (auto i = 0; i < allShaders.size(); ++i) {
    if (allShaders[i]->vertexShaderPath == vertShaderPath &&
        allShaders[i]->fragShaderPath == fragShaderPath) {
      // reload the shader if specified
      if (forceReload) {
        allShaders[i]->LoadAndCompileShader(vertShaderPath.c_str(),
                                            fragShaderPath.c_str(),
                                            geomShaderPath.c_str());
        Console.Log("[info]: reload shader at:\n%s\n%s\n", vertShaderPath.c_str(), fragShaderPath.c_str());
      }

      return allShaders[i];
    }
  }
  Shader *loadedShader = new Shader(
      vertShaderPath.c_str(), fragShaderPath.c_str(), geomShaderPath.c_str());
  loadedShader->identifier = fs::path(vertShaderPath).stem().string();
  allShaders.push_back(loadedShader);
  return loadedShader;
}

vector<Mesh *> ResourceManager::getModel(string modelPath) {
  // read file via ASSIMP
  Assimp::Importer importer;
  vector<Mesh *> meshes;
  const aiScene *scene = importer.ReadFile(
      modelPath.c_str(), aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                             aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
  // check for errors
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) // if is Not Zero
  {
    Console.Log("[error]: Assimp error: %s\n", importer.GetErrorString());
    return meshes;
  }
  // process ASSIMP's root node recursively
  processNode(scene->mRootNode, scene, meshes, modelPath);
  return meshes;
}

Mesh *ResourceManager::processMesh(aiMesh *mesh, const aiScene *scene, string &modelPath) {
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
    if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
    {
      glm::vec2 vec;
      // a vertex can contain up to 8 different texture coordinates. We thus
      // make the assumption that we won't use models where a vertex can have
      // multiple texture coordinates so we always take the first set (0).
      vec.x = mesh->mTextureCoords[0][i].x;
      vec.y = mesh->mTextureCoords[0][i].y;
      vertex.TexCoords = vec;
      // // tangent
      // vector.x = mesh->mTangents[i].x;
      // vector.y = mesh->mTangents[i].y;
      // vector.z = mesh->mTangents[i].z;
      // vertex.Tangent = vector;
      // // bitangent
      // vector.x = mesh->mBitangents[i].x;
      // vector.y = mesh->mBitangents[i].y;
      // vector.z = mesh->mBitangents[i].z;
      // vertex.Bitangent = vector;
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

  Mesh *loadedMesh = new Mesh(vertices, indices);
  loadedMesh->identifier = string(mesh->mName.C_Str());
  loadedMesh->modelPath = modelPath;

  return loadedMesh;
}

Entity *ResourceManager::GetModelEntity(string path) {
  auto meshes = GetModel(path);
  auto parentObject = Core.EManager.AddNewEntity();
  parentObject->name = fs::path(path).filename().string();
  for (auto mesh : meshes) {
    auto childObject = Core.EManager.AddNewEntity();
    parentObject->AssignChild(childObject);
    childObject->name = mesh->identifier;
    childObject->AddComponent<MeshRenderer>(mesh);
    childObject->AddComponent<Material>();
  }
  return parentObject;
}

unsigned int ResourceManager::textureFromFile(string texturePath, bool gamma) {
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
    // printf("[error]: Texture failed to load at path: %s\n",
    //             texturePath.c_str());
    Console.Log("[error]: Texture failed to load at path: %s\n",
                texturePath.c_str());
    stbi_image_free(data);
  }

  return textureID;
}

void ResourceManager::processNode(aiNode *node, const aiScene *scene,
                                  vector<Graphics::Mesh *> &meshes, string &modelPath) {
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

}; // namespace Resource