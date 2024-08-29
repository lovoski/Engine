#pragma once

#include "Global.hpp"
#include "Function/AssetsType.hpp"
#include "Function/Render/RenderPass.hpp"
#include "Function/Animation/Motion.hpp"
#include "Function/General/ComputeShader.hpp"

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
  Render::BasePass *InstantiateMaterial(std::string identifier) {
    Render::BasePass *material = new T();
    material->identifier = identifier;
    allMaterials.push_back(material);
    return material;
  }
  std::vector<std::string> GetIndetifiersForAllCachedMaterials();

  // Load cached shader with identifiers
  Render::Shader *GetShader(std::string identifier);
  // Load new shader from file path
  Render::Shader *GetShader(std::string vsp, std::string fsp,
                            std::string gsp = "none");
  // Get identifiers for all cached shaders
  std::vector<std::string> GetIdentifiersForAllCachedShaders();

  std::shared_ptr<Entity> LoadAndCreateEntityFromFile(std::string modelPath);

private:
  // path to texture
  std::map<std::string, Texture *> allTextures;
  // path to model, one model could contain multiple meshes
  std::map<std::string, std::vector<Render::Mesh *>> allMeshes;
  // from filepath to skeleton (actor)
  std::map<std::string, Animation::Skeleton *> allSkeletons;
  // identifier to shader data
  std::map<std::string, Render::Shader *> allShaders;
  // path to motion data
  std::map<std::string, Animation::Motion *> allMotions;

  std::vector<Render::BasePass *> allMaterials;

  std::vector<Render::Mesh *> loadAndCreateMeshFromFile(std::string modelPath);
};

static AssetsLoader &Loader = AssetsLoader::Ref();

}; // namespace aEngine