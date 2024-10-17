#include "Function/Render/Passes/ZenlessZoneZero.hpp"
#include "Function/AssetsLoader.hpp"
#include "Function/GUI/Helpers.hpp"
#include "Function/Render/Passes/Header.hpp"

namespace aEngine::Render {

// shader for characters from: https://github.com/m4urlclo0/ZZZ-Assets

ZenlessZoneZeroCharacterBeta::ZenlessZoneZeroCharacterBeta() {
  shader = Loader.GetShader(ASSETS_PATH "/shaders/zzz/character_beta.vs",
                            ASSETS_PATH "/shaders/zzz/character_beta.fs");
  initTextures("::null_texture", "::null_texture", "::null_texture");
}
void ZenlessZoneZeroCharacterBeta::BeforePass() {
  glDisable(GL_CULL_FACE);
  shader->SetTexture2D(D_Map, "D_Map", 0);
  shader->SetTexture2D(M_Map, "M_Map", 1);
  shader->SetTexture2D(N_Map, "N_Map", 2);
}
void ZenlessZoneZeroCharacterBeta::FinishPass() {}
void ZenlessZoneZeroCharacterBeta::DrawInspectorGUI() {
  GUIUtils::DragableTextureTarget("D Map", D_Map);
  GUIUtils::DragableTextureTarget("M Map", M_Map);
  GUIUtils::DragableTextureTarget("N Map", N_Map);
  if (ImGui::Button("Reload Shader", {-1, 30}))
    shader = Loader.GetShader(ASSETS_PATH "/shaders/zzz/character_beta.vs",
                              ASSETS_PATH "/shaders/zzz/character_beta.fs");
}

void ZenlessZoneZeroCharacterBeta::initTextures(std::string d, std::string m,
                                                std::string n) {
  nullTex = Loader.GetTexture("::null_texture")->id;
  whiteTex = Loader.GetTexture("::white_texture")->id;
  D_Map = *Loader.GetTexture(d);
  M_Map = *Loader.GetTexture(m);
  N_Map = *Loader.GetTexture(n);
}

}; // namespace aEngine::Render

REGISTER_RENDER_PASS_SL(aEngine::Render, ZenlessZoneZeroCharacterBeta)