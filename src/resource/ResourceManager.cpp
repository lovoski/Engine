#include "ResourceManager.hpp"
#include "ecs/components/Material.hpp"
#include "ecs/components/MeshRenderer.hpp"
#include "geometry/Mesh.hpp"
#include "utils/Reflection.hpp"

namespace Resource {

ResourceManager::ResourceManager() {

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
  planePrimitive->name = "plane";
  // sphere
  auto sphereMesh = GetModel(REPO_SOURCE_DIR "/src/default/primitives/sphere.obj");
  spherePrimitive = new Graphics::Mesh(sphereMesh[0]->vertices, sphereMesh[0]->indices);
  spherePrimitive->name = "sphere";
  // cube
  auto cubeMesh = GetModel(REPO_SOURCE_DIR "/src/default/primitives/cube.obj");
  cubePrimitive = new Graphics::Mesh(cubeMesh[0]->vertices, cubeMesh[0]->indices);
  cubePrimitive->name = "cube";
  // cylinder
  auto cylinderMesh = GetModel(REPO_SOURCE_DIR "/src/default/primitives/cylinder.obj");
  cylinderPrimitive = new Graphics::Mesh(cylinderMesh[0]->vertices, cylinderMesh[0]->indices);
  cylinderPrimitive->name = "cylinder";
  // cone
  auto coneMesh = GetModel(REPO_SOURCE_DIR "/src/default/primitives/cone.obj");
  conePrimitive = new Graphics::Mesh(coneMesh[0]->vertices, coneMesh[0]->indices);
  conePrimitive->name = "cone";

  // load icon library with specified order
  iconTextures.push_back(textureFromFile(REPO_SOURCE_DIR "/src/default/icons/NULL.png"));
}

ResourceManager::~ResourceManager() {
  for (int i = 0; i < meshLoaded.size(); ++i)
    delete meshLoaded[i];
  for (int i = 0; i < shaderLoaded.size(); ++i)
    delete shaderLoaded[i];
  for (auto matData : matDataLoaded)
    delete matData.second;
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
}

void ResourceManager::DumpProjectConfigFile(string projectConfigPath) {
  Json json;

  // dump project settings

  // locations to the scene files

  std::ofstream projectConfigOutput(projectConfigPath);
  if (!projectConfigOutput.is_open()) {
    cout << "error dumping project config file" << endl;
    return;
  }
  projectConfigOutput << json;
  projectConfigOutput.close();
}

void ResourceManager::LoadProjectConfigFile(string projectConfigPath) {
  Json json;
  std::ifstream projectConfigInput(projectConfigPath);
  if (!projectConfigInput.is_open()) {
    cout << "error loading project config file" << endl;
    return;
  }
  projectConfigInput >> json;
  projectConfigInput.close();

  projectRootDir = projectConfigPath.substr(0, projectConfigPath.find_last_of("\\/"));
  cout << "Load project at : " << projectRootDir << endl;
  cout << "Project settings: \n" << json << endl;
}


vector<string> ResourceManager::GetAvailableMaterials() {
  vector<string> materialIdentifiers;
  for (auto ele : matDataNameToID) {
    materialIdentifiers.push_back(ele.first);
  }
  return materialIdentifiers;
}

MaterialData *ResourceManager::GetMaterialData(string identifier) {
  if (matDataNameToID.find(identifier) == matDataNameToID.end()) {
    // create the new material data
    MaterialData *matData = new MaterialData();
    matData->identifier = matDataCounter;
    // setup the default material properties
    matData->SetDefaultMaterial();
    matDataNameToID[identifier] = matDataCounter;
    matDataLoaded[matDataCounter] = matData;
    matDataCounter++;
    return matData;
  } else {
    auto id = matDataNameToID[identifier];
    return matDataLoaded[id];
  }
}

Shader *ResourceManager::GetShader(string vertShaderPath, string fragShaderPath,
                                   bool forceReload) {
  for (auto i = 0; i < shaderLoaded.size(); ++i) {
    if (shaderLoaded[i]->vertexShaderPath == vertShaderPath &&
        shaderLoaded[i]->fragShaderPath == fragShaderPath) {
      // reload the shader if specified
      if (forceReload)
        shaderLoaded[i]->LoadAndCompileShader(vertShaderPath.c_str(),
                                              fragShaderPath.c_str());
      return shaderLoaded[i];
    }
  }
  // the shader has not been loaded
  Shader *loadedShader =
      new Shader(vertShaderPath.c_str(), fragShaderPath.c_str());
  shaderLoaded.push_back(loadedShader);
  return loadedShader;
}

Texture ResourceManager::GetTextureFromImage(string imageFilePath) {
  for (auto texture : texturesLoaded) {
    if (std::strcmp(texture.path.c_str(), imageFilePath.c_str()) == 0) {
      return texture;
    }
  }
  Texture texture;
  texture.id = textureFromFile(imageFilePath);
  texture.type = "imageTex";
  texture.path = imageFilePath;
  texturesLoaded.push_back(texture);
  return texture;
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

Mesh *ResourceManager::GetPrimitive(PRIMITIVE_TYPE pType) {
  if (pType == PRIMITIVE_TYPE::SPHERE) {
    return spherePrimitive;
  } else if (pType == PRIMITIVE_TYPE::CUBE) {
    return cubePrimitive;
  } else if (pType == PRIMITIVE_TYPE::PLANE) {
    return planePrimitive;
  } else if (pType == PRIMITIVE_TYPE::CYLINDER) {
    return cylinderPrimitive;
  } else if (pType == PRIMITIVE_TYPE::CONE) {
    return conePrimitive;
  } else return nullptr;
}

vector<Texture> ResourceManager::loadMaterialTextures(aiMaterial *mat,
                                                      aiTextureType type,
                                                      string typeName) {
  vector<Texture> textures;
  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
    aiString str;
    mat->GetTexture(type, i, &str);
    bool skip = false;
    for (unsigned int j = 0; j < texturesLoaded.size(); j++) {
      if (std::strcmp(texturesLoaded[j].path.data(), str.C_Str()) == 0) {
        // duplicate texture found
        textures.push_back(texturesLoaded[j]);
        skip = true;
        break;
      }
    }
    if (!skip) { // if texture hasn't been loaded already, load it
      Texture texture;
      texture.id = textureFromFile(str.C_Str());
      texture.type = typeName;
      texture.path = str.C_Str();
      textures.push_back(texture);
      texturesLoaded.push_back(texture);
    }
  }
  return textures;
}

Mesh *ResourceManager::processMesh(aiMesh *mesh, const aiScene *scene) {
  // data to fill
  vector<Vertex> vertices;
  vector<unsigned int> indices;

  // walk through each of the mesh's vertices
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Vertex vertex;
    glm::vec3 vector; // we declare a placeholder vector since assimp uses its
                      // own vector class that doesn't directly convert to
                      // glm's vec3 class so we transfer the data to this
                      // placeholder glm::vec3 first.
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
  // process materials
  aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
  // we assume a convention for sampler names in the shaders. Each diffuse
  // texture should be named as 'texture_diffuseN' where N is a sequential
  // number ranging from 1 to MAX_SAMPLER_NUMBER. Same applies to other
  // texture as the following list summarizes: diffuse: texture_diffuseN
  // specular: texture_specularN
  // normal: texture_normalN

  // don't load the texture with the mesh
  // // 1. diffuse maps
  // vector<Texture> diffuseMaps =
  //     loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
  // // 2. specular maps
  // vector<Texture> specularMaps = loadMaterialTextures(
  //     material, aiTextureType_SPECULAR, "texture_specular");
  // // 3. normal maps
  // std::vector<Texture> normalMaps =
  //     loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
  // // 4. height maps
  // std::vector<Texture> heightMaps =
  //     loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");

  Mesh *loadedMesh = new Mesh(vertices, indices);
  loadedMesh->name = string(mesh->mName.C_Str());
  meshLoaded.push_back(loadedMesh);

  return loadedMesh;
}

void ResourceManager::processNode(aiNode *node, const aiScene *scene,
                                  vector<Graphics::Mesh *> &meshes) {
  // process each mesh located at the current node
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    // the node object only contains indices to index the actual objects in
    // the scene. the scene contains all the data, node is just to keep stuff
    // organized (like relations between nodes).
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(processMesh(mesh, scene));
  }
  // after we've processed all of the meshes (if any) we then recursively
  // process each of the children nodes
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene, meshes);
  }
}

Entity *ResourceManager::GetModelEntity(string path) {
  auto meshes = GetModel(path);
  if (meshes.size() > 0) {
    // flatten all meshes to a common parent
    auto parentObject = ECS::EManager.AddNewEntity();
    parentObject->name = path.substr(path.find_last_of("/\\") + 1);
    parentObject->AddComponent<BaseMaterial>();
    // the render system only renders the entities with MeshRenderer component
    // in an list with no order (TODO: add order of rendering)
    for (auto mesh : meshes) {
      auto childObject = ECS::EManager.AddNewEntity();
      childObject->name = mesh->name;
      childObject->AddComponent<MeshRenderer>(mesh);
      childObject->AddComponent<BaseMaterial>();
      parentObject->AssignChild(childObject);
    }
    return parentObject;
  } else
    return nullptr;
}

Entity *ResourceManager::GetPrimitiveEntity(PRIMITIVE_TYPE pType) {
  auto primitiveObject = ECS::EManager.AddNewEntity();
  primitiveObject->AddComponent<BaseMaterial>();
  if (pType == PRIMITIVE_TYPE::CUBE) {
    primitiveObject->AddComponent<MeshRenderer>(cubePrimitive);
    primitiveObject->name = "Cube";
  } else if (pType == PRIMITIVE_TYPE::SPHERE) {
    primitiveObject->AddComponent<MeshRenderer>(spherePrimitive);
    primitiveObject->name = "Sphere";
  } else if (pType == PRIMITIVE_TYPE::PLANE) {
    primitiveObject->AddComponent<MeshRenderer>(planePrimitive);
    primitiveObject->name = "Plane";
  } else if (pType == PRIMITIVE_TYPE::CYLINDER) {
    primitiveObject->AddComponent<MeshRenderer>(cylinderPrimitive);
    primitiveObject->name = "Cylinder";
  } else if (pType == PRIMITIVE_TYPE::CONE) {
    primitiveObject->AddComponent<MeshRenderer>(conePrimitive);
    primitiveObject->name = "Cone";
  } else return nullptr;
  return primitiveObject;
}

vector<Mesh *> ResourceManager::GetModel(string modelPath) {
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
  processNode(scene->mRootNode, scene, meshes);
  return meshes;
}

}; // namespace Resource