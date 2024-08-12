#pragma once

#include "Global.hpp"
#include "Utils/AssetsType.hpp"

#include "Utils/Animation/Motion.hpp"

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
  std::vector<Render::Mesh *> GetModel(std::string modelPath);
  Render::MaterialData *GetMaterial(std::string materialPath);
  Animation::Motion *GetMotion(std::string motionPath);
private:
  // path to texture
  std::map<std::string, Texture*> allTextures;
  // path to model, one model could contain multiple meshes
  std::map<std::string, std::vector<Render::Mesh*>> allMeshes;
  // path to material data
  std::map<std::string, Render::MaterialData*> allMaterials;
  // path to motion data
  std::map<std::string, Animation::Motion*> allMotions;
};

static AssetsLoader &Loader = AssetsLoader::Ref();

};