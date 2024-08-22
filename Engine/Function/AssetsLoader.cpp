#include "Component/Animator.hpp"
#include "Component/MeshRenderer.hpp"
#include "Component/NativeScript.hpp"

#include "Function/AssetsLoader.hpp"
#include "Function/General/Deformers.hpp"
#include "Function/Render/MaterialData.hpp"
#include "Function/Render/Mesh.hpp"
#include "Function/Render/Shader.hpp"

#include <fbxsdk.h>

#include "Scene.hpp"

namespace aEngine {

using std::string;
using std::vector;

struct BoneInfo {
  std::string boneName;
  glm::mat4 offsetMatrix;
  glm::vec3 localPosition = glm::vec3(0.0f);
  glm::quat localRotation = glm::quat(1.0f, glm::vec3(0.0f));
  glm::vec3 localScale = glm::vec3(1.0f);
  int parentIndex =
      -1; // Index of the parent bone in the hierarchy (-1 if root)
  std::vector<int> children; // Indices of child bones
};

int boneCounter = 0;
std::map<std::string, int> boneInfoMap; // Map bone name to BoneInfo
std::vector<BoneInfo> boneInfos;

AssetsLoader::AssetsLoader() {
  // stb image library setup
  stbi_set_flip_vertically_on_load(true);
}

AssetsLoader::~AssetsLoader() {
  for (auto texture : allTextures) {
    if (texture.second) {
      glDeleteTextures(1, &texture.second->id);
      delete texture.second;
    }
  }
  for (auto meshEle : allMeshes) {
    for (auto mesh : meshEle.second)
      if (mesh)
        delete mesh;
  }
  for (auto shader : allShaders) {
    if (shader.second)
      delete shader.second;
  }
}

unsigned int loadAndCreateTextureFromFile(string texturePath);
vector<Render::Mesh *> loadAndCreateMeshFromFile(string modelPath);

void AssetsLoader::LoadDefaultAssets() {
  // initialize all the primitives
  // plane
  vector<Vertex> vertices;
  vector<unsigned int> indices;
  vertices.push_back({{0.5f, 0.0f, 0.5f, 1.0f},
                      {0.0, 1.0, 0.0, 0.0f},
                      {1.0f, 1.0f, 1.0f, 1.0f}});
  vertices.push_back({{0.5f, 0.0f, -0.5f, 1.0f},
                      {0.0, 1.0, 0.0, 0.0f},
                      {1.0f, 0.0f, 1.0f, 1.0f}});
  vertices.push_back({{-0.5f, 0.0f, -0.5f, 1.0f},
                      {0.0, 1.0, 0.0, 0.0f},
                      {0.0f, 0.0f, 1.0f, 1.0f}});
  vertices.push_back({{-0.5f, 0.0f, 0.5f, 1.0f},
                      {0.0, 1.0, 0.0, 0.0f},
                      {0.0f, 1.0f, 1.0f, 1.0f}});
  indices = {0, 1, 3, 1, 2, 3};
  auto planePrimitive = new Render::Mesh(vertices, indices);
  planePrimitive->identifier = "plane";
  planePrimitive->modelPath = "::planePrimitive";
  allMeshes.insert(std::make_pair("::planePrimitive",
                                  vector<Render::Mesh *>({planePrimitive})));
  // sphere
  auto sphereMesh = loadAndCreateMeshFromFile("./Assets/meshes/sphere.fbx");
  allMeshes.insert(std::make_pair("::spherePrimitive", sphereMesh));
  // cube
  auto cubeMesh = loadAndCreateMeshFromFile("./Assets/meshes/cube.fbx");
  allMeshes.insert(std::make_pair("::cubePrimitive", cubeMesh));
  // cylinder
  auto cylinderMesh = loadAndCreateMeshFromFile("./Assets/meshes/cylinder.fbx");
  allMeshes.insert(std::make_pair("::cylinderPrimitive", cylinderMesh));
  // cone
  auto coneMesh = loadAndCreateMeshFromFile("./Assets/meshes/cone.fbx");
  allMeshes.insert(std::make_pair("::conePrimitive", coneMesh));

  // load icons
  Texture *nullIcon = new Texture();
  nullIcon->id = loadAndCreateTextureFromFile("./Assets/icons/NULL.png");
  nullIcon->path = "::NULL_ICON";
  allTextures.insert(std::make_pair("::NULL_ICON", nullIcon));

  // load shaders
  Render::Shader *diffuseShader = new Render::Shader();
  diffuseShader->identifier = "::diffuse";
  diffuseShader->LoadAndRecompileShaderSource(Render::diffuseVS,
                                              Render::diffuseFS);
  allShaders.insert(std::make_pair("::diffuse", diffuseShader));

  ComputeShader *skeletonAnimDeform = new ComputeShader(skinnedMeshDeform);
  skeletonAnimDeform->identifier = "::skinnedMeshDeform";
  allComputeShaders.insert(
      std::make_pair(skeletonAnimDeform->identifier, skeletonAnimDeform));

  Render::Shader *errorShader = new Render::Shader();
  errorShader->identifier = "::error";
  errorShader->LoadAndRecompileShaderSource(Render::errorVS, Render::errorFS);
  allShaders.insert(std::make_pair("::error", errorShader));
}

std::vector<std::string> AssetsLoader::GetIndetifiersForAllCachedMaterials() {
  std::vector<std::string> result;
  for (auto m : allMaterials)
    result.push_back(m->identifier);
  return result;
}

std::vector<std::string> AssetsLoader::GetIdentifiersForAllCachedShaders() {
  std::vector<std::string> result;
  for (auto s : allShaders) {
    result.push_back(s.first);
  }
  return result;
}
Render::Shader *AssetsLoader::GetShader(std::string identifier) {
  auto s = allShaders.find(identifier);
  if (s == allShaders.end()) {
    Console.Log("[error]: shader with identifier %s not found\n",
                identifier.c_str());
    return GetShader("::error");
  } else {
    Console.Log("[info]: get shader with identifier %s\n", identifier.c_str());
    return (*s).second;
  }
}
Render::Shader *AssetsLoader::GetShader(std::string vsp, std::string fsp,
                                        std::string gsp) {
  Render::Shader *newShader = new Render::Shader();
  if (newShader->LoadAndRecompileShader(vsp, fsp, gsp)) {
    // create identifier for this shader
    newShader->identifier = fs::path(vsp).stem().string() + ":" +
                            fs::path(fsp).stem().string() + ":" +
                            fs::path(gsp).stem().string();
    allShaders.insert(std::make_pair(newShader->identifier, newShader));
    Console.Log("[info]: load shader from path, identifier as %s vsp=%s, "
                "fsp=%s, gsp=%s\n",
                newShader->identifier.c_str(), vsp.c_str(), fsp.c_str(),
                gsp.c_str());
    return newShader;
  } else
    return GetShader(":error");
}
ComputeShader *AssetsLoader::GetLoadedComputeShader(std::string identifier) {
  auto s = allComputeShaders.find(identifier);
  if (s == allComputeShaders.end()) {
    Console.Log("[error]: shader with identifier %s not found\n",
                identifier.c_str());
    return nullptr;
  } else {
    Console.Log("[info]: get shader with identifier %s\n", identifier.c_str());
    return (*s).second;
  }
}

Animation::Motion *AssetsLoader::GetMotion(std::string motionPath) {
  if (allMotions.find(motionPath) == allMotions.end()) {
    // load new motion
    Animation::Motion *motion = new Animation::Motion();
    std::string extension = fs::path(motionPath).extension().string();
    if (extension == ".bvh") {
      Console.Log("[info]: load motion data from %s\n", motionPath.c_str());
      motion->LoadFromBVH(motionPath);
      allMotions.insert(std::make_pair(motionPath, motion));
      return motion;
    } else {
      Console.Log("[error]: unnsupported motion file extension %s\n",
                  extension.c_str());
      return nullptr;
    }
  } else {
    Console.Log("[info]: get cached motion %s\n", motionPath.c_str());
    return allMotions[motionPath];
  }
}

Texture *AssetsLoader::GetTexture(string texturePath) {
  if (allTextures.find(texturePath) == allTextures.end()) {
    // load new texture
    Texture *newTexture = new Texture();
    newTexture->id = loadAndCreateTextureFromFile(texturePath);
    newTexture->path = texturePath;
    allTextures[texturePath] = newTexture;
    return newTexture;
  } else {
    return allTextures[texturePath];
  }
}

std::vector<Render::Mesh *> AssetsLoader::GetModel(std::string modelPath) {
  std::vector<Render::Mesh *> result;
  if (allMeshes.find(modelPath) == allMeshes.end()) {
    // load new model
    auto modelMeshes = loadAndCreateMeshFromFile(modelPath);
    if (modelMeshes.size() == 0) {
      Console.Log("[error]: the file %s has no mesh\n", modelPath.c_str());
      return result;
    }
    Console.Log("[info]: load model at %s\n", modelPath.c_str());
    allMeshes[modelPath] = modelMeshes;
    return modelMeshes;
  } else {
    auto meshes = allMeshes[modelPath];
    if (modelPath[0] == ':') {
      Console.Log("[info]: load primitive %s\n", modelPath.c_str());
    } else
      Console.Log("[info]: get cached model %s\n", modelPath.c_str());
    return meshes;
  }
}

Render::Mesh *AssetsLoader::GetMesh(string modelPath, string identifier) {
  if (allMeshes.find(modelPath) == allMeshes.end()) {
    // load new model
    auto modelMeshes = loadAndCreateMeshFromFile(modelPath);
    if (modelMeshes.size() == 0) {
      Console.Log("[error]: the file %s has no mesh\n", modelPath.c_str());
      return nullptr;
    }
    for (auto mesh : modelMeshes) {
      if (mesh->identifier == identifier) {
        Console.Log("[info]: load model at %s, get mesh named %s\n",
                    modelPath.c_str(), identifier.c_str());
        return mesh;
      }
    }
    Console.Log("[error]: load model at %s, but no mesh named %s was found\n",
                modelPath.c_str(), identifier.c_str());
    return nullptr;
  } else {
    auto meshes = allMeshes[modelPath];
    if (modelPath[0] == ':') {
      Console.Log("[info]: load primitive %s\n", modelPath.c_str());
      return meshes[0];
    }
    for (auto mesh : meshes) {
      if (mesh->identifier == identifier) {
        Console.Log("[info]: get cached mesh named %s from model %s\n",
                    identifier.c_str(), modelPath.c_str());
        return mesh;
      }
    }
    Console.Log(
        "[error]: model %s has been loaded, but no mesh named %s found\n",
        modelPath.c_str(), identifier.c_str());
    return nullptr;
  }
}

unsigned int loadAndCreateTextureFromFile(string texturePath) {
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
    Console.Log("[error]: Texture failed to load at path: %s\n",
                texturePath.c_str());
    stbi_image_free(data);
  }

  return textureID;
}

Entity *AssetsLoader::LoadAndCreateEntityFromFile(string modelPath) {
  // clean up globa variables
  boneCounter = 0;
  boneInfos.clear();
  boneInfoMap.clear();

  auto globalParent = GWORLD.AddNewEntity();
  globalParent->name = fs::path(modelPath).filename().stem().string();

  return globalParent;
}

Render::Mesh *ProcessMesh(fbxsdk::FbxMesh *mesh) {
  // Triangulate the model if needed
  if (!mesh->IsTriangleMesh()) {
    fbxsdk::FbxGeometryConverter geometryConverter(
        mesh->GetNode()->GetScene()->GetFbxManager());
    mesh = static_cast<fbxsdk::FbxMesh *>(geometryConverter.Triangulate(mesh, true));
  }

  // Get the number of control points (vertices) in the mesh
  int controlPointCount = mesh->GetControlPointsCount();
  fbxsdk::FbxVector4 *controlPoints = mesh->GetControlPoints();

  vector<Vertex> meshVertices;
  vector<unsigned int> meshIndices;

  // Mapping from a control point to the corresponding vertex index
  std::unordered_map<int, int> controlPointToVertexIndex;

  // Extract vertex positions, normals, and texture coordinates
  fbxsdk::FbxGeometryElementNormal *normalElement = mesh->GetElementNormal();
  fbxsdk::FbxGeometryElementUV *uvElement = mesh->GetElementUV();

  for (int i = 0; i < mesh->GetPolygonCount(); i++) {
    for (int j = 0; j < mesh->GetPolygonSize(i); j++) {
      int controlPointIndex = mesh->GetPolygonVertex(i, j);

      // If this control point hasn't been mapped to a vertex yet
      if (controlPointToVertexIndex.find(controlPointIndex) ==
          controlPointToVertexIndex.end()) {
        // Create a new vertex
        Vertex vertex;

        // Set position
        fbxsdk::FbxVector4 controlPoint = controlPoints[controlPointIndex];
        vertex.Position.x = controlPoint[0];
        vertex.Position.y = controlPoint[1];
        vertex.Position.z = controlPoint[2];
        vertex.Position.w = 1.0f;

        // Set normal
        if (normalElement) {
          fbxsdk::FbxVector4 normal;
          if (normalElement->GetMappingMode() ==
              fbxsdk::FbxGeometryElement::eByControlPoint) {
            if (normalElement->GetReferenceMode() ==
                fbxsdk::FbxGeometryElement::eDirect) {
              normal = normalElement->GetDirectArray().GetAt(controlPointIndex);
            } else if (normalElement->GetReferenceMode() ==
                       fbxsdk::FbxGeometryElement::eIndexToDirect) {
              int normalIndex =
                  normalElement->GetIndexArray().GetAt(controlPointIndex);
              normal = normalElement->GetDirectArray().GetAt(normalIndex);
            }
          } else if (normalElement->GetMappingMode() ==
                     fbxsdk::FbxGeometryElement::eByPolygonVertex) {
            int normalIndex = mesh->GetPolygonVertexIndex(i) + j;
            if (normalElement->GetReferenceMode() ==
                fbxsdk::FbxGeometryElement::eDirect) {
              normal = normalElement->GetDirectArray().GetAt(normalIndex);
            } else if (normalElement->GetReferenceMode() ==
                       fbxsdk::FbxGeometryElement::eIndexToDirect) {
              normalIndex = normalElement->GetIndexArray().GetAt(normalIndex);
              normal = normalElement->GetDirectArray().GetAt(normalIndex);
            }
          }
          vertex.Normal.x = normal[0];
          vertex.Normal.y = normal[1];
          vertex.Normal.z = normal[2];
          vertex.Normal.w = 0.0f;
        }

        // Set UV coordinates
        if (uvElement) {
          fbxsdk::FbxVector2 uv;
          if (uvElement->GetMappingMode() ==
              fbxsdk::FbxGeometryElement::eByControlPoint) {
            if (uvElement->GetReferenceMode() == fbxsdk::FbxGeometryElement::eDirect) {
              uv = uvElement->GetDirectArray().GetAt(controlPointIndex);
            } else if (uvElement->GetReferenceMode() ==
                       fbxsdk::FbxGeometryElement::eIndexToDirect) {
              int uvIndex = uvElement->GetIndexArray().GetAt(controlPointIndex);
              uv = uvElement->GetDirectArray().GetAt(uvIndex);
            }
          } else if (uvElement->GetMappingMode() ==
                     fbxsdk::FbxGeometryElement::eByPolygonVertex) {
            int uvIndex = mesh->GetTextureUVIndex(i, j);
            uv = uvElement->GetDirectArray().GetAt(uvIndex);
          }
          vertex.TexCoords.x = uv[0];
          vertex.TexCoords.y = uv[1];
          vertex.TexCoords.z = 0.0f;
          vertex.TexCoords.w = 0.0f;
        }

        // Initialize bone weights and indices
        for (int b = 0; b < MAX_BONES; ++b) {
          vertex.BoneId[b] = 0;
          vertex.BoneWeight[b] = 0.0f;
        }

        // Add the vertex to the mesh
        int vertexIndex = meshVertices.size();
        meshVertices.push_back(vertex);

        // Map the control point index to the vertex index
        controlPointToVertexIndex[controlPointIndex] = vertexIndex;
      }

      // Get the vertex index
      int vertexIndex = controlPointToVertexIndex[controlPointIndex];

      // Add the index to the mesh indices
      meshIndices.push_back(vertexIndex);
    }
  }

  // Create and return the mesh
  auto result = new Render::Mesh(meshVertices, meshIndices);
  result->identifier = mesh->GetNode()->GetName();
  return result;
}

void TraverseNode(fbxsdk::FbxNode *node, vector<Render::Mesh *> &meshes) {
  if (node) {
    fbxsdk::FbxMesh *mesh = node->GetMesh();
    if (mesh) {
      meshes.push_back(ProcessMesh(mesh));
    }
    for (int i = 0; i < node->GetChildCount(); i++) {
      TraverseNode(node->GetChild(i), meshes);
    }
  }
}

vector<Render::Mesh *> loadAndCreateMeshFromFile(string modelPath) {
  boneCounter = 0;
  boneInfos.clear();
  boneInfoMap.clear();

  vector<Render::Mesh *> meshes;

  // Initialize the FBX SDK manager
  fbxsdk::FbxManager *sdkManager = fbxsdk::FbxManager::Create();

  // Create an IOSettings object
  fbxsdk::FbxIOSettings *ioSettings =
      fbxsdk::FbxIOSettings::Create(sdkManager, IOSROOT);
  sdkManager->SetIOSettings(ioSettings);

  // Create an FBX scene
  fbxsdk::FbxScene *scene = fbxsdk::FbxScene::Create(sdkManager, "MyScene");

  // Create an importer
  fbxsdk::FbxImporter *importer = FbxImporter::Create(sdkManager, "");

  // Initialize the importer with the path to the OBJ file
  if (!importer->Initialize(modelPath.c_str(), -1,
                            sdkManager->GetIOSettings())) {
    std::cerr << "Failed to initialize importer: "
              << importer->GetStatus().GetErrorString() << std::endl;
    return meshes;
  }

  // Import the scene
  if (!importer->Import(scene)) {
    std::cerr << "Failed to import scene: "
              << importer->GetStatus().GetErrorString() << std::endl;
    return meshes;
  }

  // Destroy the importer since it's no longer needed
  importer->Destroy();

  // Traverse the scene to process each mesh
  fbxsdk::FbxNode *rootNode = scene->GetRootNode();
  if (rootNode) {
    TraverseNode(rootNode, meshes);
  }

  // Destroy the SDK manager and all associated objects
  sdkManager->Destroy();

  return meshes;
}

}; // namespace aEngine