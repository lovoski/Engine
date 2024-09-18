#pragma once

#include "Function/Animation/Motion.hpp"
#include "Function/AssetsType.hpp"
#include "Function/General/ComputeShader.hpp"
#include "Function/Render/RenderPass.hpp"
#include "Global.hpp"

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

  // load and cache a texture from file, returns "::null_texture" if
  // failed to load.
  Texture *GetTexture(std::string texturePath, bool flipVertically = true);
  Texture *GetHDRTexture(std::string texturePath, bool flipVertically = true);
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
  std::shared_ptr<Render::Shader> GetShader(std::string identifier);
  // Load new shader from file path
  std::shared_ptr<Render::Shader> GetShader(std::string vsp, std::string fsp,
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
  std::map<std::string, std::shared_ptr<Render::Shader>> allShaders;
  // path to motion data
  std::map<std::string, Animation::Motion *> allMotions;

  std::vector<Render::BasePass *> allMaterials;

  void loadFBXModelFile(std::vector<Render::Mesh *> &meshes,
                        std::string modelPath);
  void loadOBJModelFile(std::vector<Render::Mesh *> &meshes,
                        std::string modelPath);
  std::vector<Render::Mesh *>
  loadAndCreateAssetsFromFile(std::string modelPath);

  void prepareDefaultShader(std::string vs, std::string fs, std::string gs,
                            std::string identifier);
};

static AssetsLoader &Loader = AssetsLoader::Ref();

}; // namespace aEngine