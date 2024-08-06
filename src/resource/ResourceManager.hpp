#pragma once

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

  // get a mesh from its model path and an identifier
  Mesh *GetMesh(string path, string identifier = "");
  // get the collection of mesh from its model path
  vector<Mesh *> GetModel(string path);

  Entity *GetModelEntity(string path);

  // materials are identified by its file path
  // use path '::base' to get the instance of the default material
  MaterialData *GetMaterialData(string path);
  // get an image texture from file, if the path matches
  Texture *GetTexture(string path, bool forceReload = false);
  Texture *GetIcon(ICON_TYPE iconType) { return allIcons[iconType]; }

  Json GetDefaultMaterialJson() {
    Json json;
    allMaterials["::base"]->Serialize(json);
    return json;
  }

  Texture *TextureSlot;

private:

  // path to texture
  std::map<string, Texture*> allTextures;
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

  Graphics::Mesh *processMesh(aiMesh *, const aiScene *, string &);

  void processNode(aiNode *, const aiScene *, vector<Graphics::Mesh *> &, string &);

  // returns the plane mesh data of a model
  vector<Mesh *> getModel(string path);

};

}; // namespace Resource