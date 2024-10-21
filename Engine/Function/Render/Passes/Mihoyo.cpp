#include "Function/Render/Passes/Mihoyo.hpp"
#include "Function/AssetsLoader.hpp"
#include "Function/GUI/Helpers.hpp"
#include "Function/Render/Passes/Header.hpp"

namespace aEngine::Render {

StarRailCharacter::StarRailCharacter() {
  shader = Loader.GetShader(ASSETS_PATH "/shaders/starrail/character.vs",
                            ASSETS_PATH "/shaders/starrail/character.fs");
  initTextures("::null_texture", "::null_texture", "::null_texture");
}
void StarRailCharacter::BeforePass() {
  glDisable(GL_CULL_FACE);
  shader->SetTexture2D(diffuse.id, "DiffuseMap", 0);
  shader->SetTexture2D(lightmap.id, "LightMap", 1);
  shader->SetTexture2D(ramptex.id, "RampTex", 2);
}
void StarRailCharacter::FinishPass() {}
void StarRailCharacter::DrawInspectorGUI() {
  GUIUtils::DragableTextureTarget("Diffuse Map", diffuse);
  GUIUtils::DragableTextureTarget("Light Map", lightmap);
  GUIUtils::DragableTextureTarget("Ramp Tex", ramptex);
  if (ImGui::Button("Reload Shader", {-1, 30}))
    shader = Loader.GetShader(ASSETS_PATH "/shaders/starrail/character.vs",
                              ASSETS_PATH "/shaders/starrail/character.fs");
}

void StarRailCharacter::initTextures(std::string d, std::string l,
                                     std::string r) {
  nullTex = Loader.GetTexture("::null_texture")->id;
  whiteTex = Loader.GetTexture("::white_texture")->id;
  diffuse = *Loader.GetTexture(d);
  lightmap = *Loader.GetTexture(l);
  ramptex = *Loader.GetTexture(r);
}

}; // namespace aEngine::Render

REGISTER_RENDER_PASS_SL(aEngine::Render, StarRailCharacter)