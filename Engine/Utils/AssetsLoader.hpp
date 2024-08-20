#pragma once

#include "Global.hpp"
#include "Utils/AssetsType.hpp"
#include "Utils/Render/MaterialData.hpp"
#include "Utils/Animation/Motion.hpp"
#include "Utils/General/ComputeShader.hpp"

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
  Animation::Motion *GetMotion(std::string motionPath);

  // Create a new instance of this material by the type,
  // cache it in an internal array
  template <typename T>
  Render::BaseMaterial *InstatiateMaterial(std::string identifier) {
    Render::BaseMaterial *material = new T();
    material->identifier = identifier;
    allMaterials.push_back(material);
    return material;
  }
  std::vector<std::string> GetIndetifiersForAllCachedMaterials();

  ComputeShader *GetLoadedComputeShader(std::string identifier);
  // Load cached shader with identifiers
  Render::Shader *GetShader(std::string identifier);
  // Load new shader from file path
  Render::Shader *GetShader(std::string vsp, std::string fsp,
                            std::string gsp = "none");
  // Get identifiers for all cached shaders
  std::vector<std::string> GetIdentifiersForAllCachedShaders();

private:
  // path to texture
  std::map<std::string, Texture *> allTextures;
  // path to model, one model could contain multiple meshes
  std::map<std::string, std::vector<Render::Mesh *>> allMeshes;
  // identifier to shader data
  std::map<std::string, Render::Shader *> allShaders;
  // identifier to compute shader
  std::map<std::string, ComputeShader *> allComputeShaders;
  // path to motion data
  std::map<std::string, Animation::Motion *> allMotions;

  std::vector<Render::BaseMaterial *> allMaterials;
};

static AssetsLoader &Loader = AssetsLoader::Ref();

}; // namespace aEngine