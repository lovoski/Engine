/**
 * Shaders for mihoyo characters.
 */
#pragma once

#include "Function/Render/RenderPass.hpp"
#include "Function/Render/VisUtils.hpp"

namespace aEngine::Render {

class StarRailCharacter : public BasePass {
public:
  StarRailCharacter();

  std::string getInspectorWindowName() override { return "Honkai Railway"; }
  void FinishPass() override;

  template <typename Archive> void save(Archive &ar) const {
    ar(diffuse.path, lightmap.path, ramptex.path);
  }
  template <typename Archive> void load(Archive &ar) {
    std::string d, l, r;
    ar(d, l, r);
    initTextures(d, l, r);
  }

  Texture diffuse, lightmap, ramptex;

private:
  // void initTextures(std::string d, std::string m, std::string n);
  unsigned int nullTex, whiteTex;
  void initTextures(std::string d, std::string l, std::string r);
  void BeforePass() override;
  void DrawInspectorGUI() override;
};

}; // namespace aEngine::Render
