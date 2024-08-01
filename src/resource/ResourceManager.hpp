#pragma once

#include "EngineConfig.h"
#include "ResourceTypes.hpp"
#include "Shader.hpp"
#include "ecs/ecs.hpp"
#include "MaterialData.hpp"
#include "ecs/systems/render/Mesh.hpp"

namespace Resource {

using Graphics::Mesh;

class ResourceManager {
public:
  ResourceManager();
  ResourceManager(ResourceManager &) = delete;
  const ResourceManager &operator=(ResourceManager &) = delete;
  ~ResourceManager();

  void Initialize();

  // returns the plane mesh data of a primitive
  Mesh *GetPrimitive(PRIMITIVE_TYPE pType);
  // get a mesh from its model path and an identifier
  Mesh *GetMesh(string path, string identifier);
  // get the collection of mesh from its model path
  vector<Mesh *> GetModel(string path);

  Entity *GetModelEntity(string path);

  // shaders are identified by their vertex and fragment shader path
  Shader *GetShader(string vertShaderPath, string fragShaderPath, string geomShaderPath = "none", bool forceReload=false);
  // materials are identified by a string identifier
  MaterialData *GetMaterialData(string path);
  // get an image texture from file, if the path matches
  Texture *GetTexture(string path, bool forceReload = false);
  Texture *GetIcon(ICON_TYPE iconType) { return allIcons[iconType]; }

  string GetProjectRootDir() { return projectRootDir; }

  Json GetDefaultMaterialJson() {
    Json json;
    allMaterials["::base"]->Serialize(json);
    return json;
  }

private:

  // path to texture
  std::map<string, Texture*> allTextures;
  // collection of shaders
  vector<Shader*> allShaders;
  // path to model, one model could contain multiple meshes
  std::map<string, vector<Mesh*>> allMeshes;
  // path to material data
  std::map<string, MaterialData*> allMaterials;
  // icon type to icon texture id
  std::map<ICON_TYPE, Texture*> allIcons;

  Mesh *cubePrimitive;
  Mesh *planePrimitive;
  Mesh *spherePrimitive;
  Mesh *cylinderPrimitive;
  Mesh *conePrimitive;

  unsigned int textureFromFile(string texturePath, bool gamma = false);

  Graphics::Mesh *processMesh(aiMesh *, const aiScene *);

  void processNode(aiNode *, const aiScene *, vector<Graphics::Mesh *> &);

  // returns the plane mesh data of a model
  vector<Mesh *> getModel(string path);

  // the project settings

  string projectRootDir = REPO_SOURCE_DIR "/assets";
};

}; // namespace Resource