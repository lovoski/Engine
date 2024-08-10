#pragma once

#include "Global.hpp"
#include "Utils/AssetsType.hpp"

namespace aEngine {

class AssetsLoader {
public:
  AssetsLoader();
  ~AssetsLoader();

  AssetsLoader(AssetsLoader &) = delete;
  const AssetsLoader &operator=(AssetsLoader &) = delete;

  // Load default assets
  void LoadDefaultAssets();

  static AssetsLoader &Ref() {
    static AssetsLoader reference;
    return reference;
  }

  Texture *GetTexture(std::string texturePath);
  Render::Mesh *GetMesh(std::string modelPath, std::string identifier);
  Render::MaterialData *GetMaterial(std::string materialPath);
private:
  // path to texture
  std::map<std::string, Texture*> allTextures;
  // path to model, one model could contain multiple meshes
  std::map<std::string, std::vector<Render::Mesh*>> allMeshes;
  // path to material data
  std::map<std::string, Render::MaterialData*> allMaterials;
};

static AssetsLoader &Loader = AssetsLoader::Ref();

};