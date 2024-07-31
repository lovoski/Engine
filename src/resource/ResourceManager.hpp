#pragma once

#include "EngineConfig.h"
#include "ResourceTypes.hpp"
#include "Shader.hpp"
#include "ecs/ecs.hpp"
#include "ecs/systems/render/Mesh.hpp"

namespace Resource {

using Graphics::Mesh;

class ResourceManager {
public:
  ResourceManager();
  ResourceManager(ResourceManager &) = delete;
  const ResourceManager &operator=(ResourceManager &) = delete;
  ~ResourceManager();

  static ResourceManager &Ref() {
    static ResourceManager reference;
    return reference;
  }

  // returns the plane mesh data of a primitive
  Mesh *GetPrimitive(PRIMITIVE_TYPE pType);
  // returns the plane mesh data of a model
  vector<Mesh *> GetModel(string);

  // create render ready primitive
  Entity *GetModelEntity(string path);
  // create render ready primitive
  Entity *GetPrimitiveEntity(PRIMITIVE_TYPE pType);
  // shaders are identified by their vertex and fragment shader path
  Shader *GetShader(string vertShaderPath, string fragShaderPath, bool forceReload=false);
  // materials are identified by a string identifier
  MaterialData *GetMaterialData(string identifier);

  // get all the identifiers of available materials
  vector<string> GetAvailableMaterials();

  string GetProjectRootDir() { return projectRootDir; }

  vector<Texture> GetAllLoadedTextures() { return texturesLoaded; }
  vector<Mesh *> GetAllLoadedMeshes() { return meshLoaded; }
  vector<Shader *> GetAllLoadedShaders() { return shaderLoaded; }

  Texture GetTextureFromImage(string imageFilePath);

  void DumpProjectConfigFile(string projectConfigPath);
  void LoadProjectConfigFile(string projectConfigPath);

  unsigned int GetIcon(ICON_TYPE iconType) { return iconTextures[(unsigned int)iconType]; }

private:
  // stores all the loaded textures
  vector<Texture> texturesLoaded;
  // stores all the loaded shaders
  vector<Shader *> shaderLoaded;
  // stores all the loaded meshes
  vector<Mesh *> meshLoaded;
  // stores all the loaded material data
  std::map<unsigned int, MaterialData *> matDataLoaded;

  Mesh *cubePrimitive;
  Mesh *planePrimitive;
  Mesh *spherePrimitive;

  unsigned int textureFromFile(string texturePath, bool gamma = false);

  vector<Texture> loadMaterialTextures(aiMaterial *, aiTextureType, string);

  Graphics::Mesh *processMesh(aiMesh *, const aiScene *);

  void processNode(aiNode *, const aiScene *, vector<Graphics::Mesh *> &);

  unsigned int matDataCounter = 0;
  std::map<string, unsigned int> matDataNameToID;

  // the project settings

  string projectRootDir = REPO_SOURCE_DIR "/assets";

  vector<unsigned int> iconTextures;

};

static ResourceManager &RManager = ResourceManager::Ref();

}; // namespace Resource