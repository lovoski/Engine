#pragma once

#include "Function/Render/RenderPass.hpp"
#include "Function/Render/VisUtils.hpp"

namespace aEngine::Render {

// Main render pass for character from zenless zone zero
// during beta test, assets from: https://github.com/m4urlclo0/ZZZ-Assets
class ZenlessZoneZeroCharacterBeta : public BasePass {
public:
  ZenlessZoneZeroCharacterBeta();

  std::string getInspectorWindowName() override { return "Zenless Zone Zero"; }
  void FinishPass() override;

  template <typename Archive> void save(Archive &ar) const {
    ar(D_Map.path, M_Map.path, N_Map.path);
  }
  template <typename Archive> void load(Archive &ar) {
    std::string p1, p2, p3;
    ar(p1, p2, p3);
    initTextures(p1, p2, p3);
  }

  Texture D_Map, M_Map, N_Map;

private:
  void initTextures(std::string d, std::string m, std::string n);
  unsigned int nullTex, whiteTex;
  void BeforePass() override;
  void DrawInspectorGUI() override;
};

}; // namespace aEngine::Render
