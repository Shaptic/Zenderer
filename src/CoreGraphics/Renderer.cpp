#include "Zenderer/CoreGraphics/Renderer.hpp"

using namespace zen;
using gfxcore::zRenderer;

// Despite the fact that it is guaranteed to exist, we define it as a pointer
// because we do not know the asset manager instance that will be used in the
// constructor, as that is determined by the user.
gfx::zMaterial*         zRenderer::s_DefaultMaterial = nullptr;

gfxcore::zVertexArray   zRenderer::s_FullscreenQuad(GL_STATIC_DRAW);
gfxcore::BlendFunc      zRenderer::s_LastBlend = gfxcore::BlendFunc::DISABLE_BLEND;
math::mat4_t       zRenderer::s_ProjMatrix;
bool                    zRenderer::s_blend = false;
bool                    zRenderer::s_wf = false;

bool zRenderer::Init(asset::zAssetManager& Assets,
                     const uint16_t w, const uint16_t h)
{
    // Load the default effect.
    s_DefaultMaterial = new gfx::zMaterial(Assets);
    gfx::zEffect& E = s_DefaultMaterial->GetEffect();
    E.Enable();
    E.SetModelMatrix(math::mat4x4_t())
    E.SetProjectionMatrix(s_ProjMatrix);
    E.Disable();

    // Load a quad into the VAO.
    gfxcore::DrawBatch D;
    gfxcore::index_t  i[] = {0, 1, 3, 3, 1, 2};
    gfxcore::vertex_t v[6];

    D.Vertices  = v;
    D.vcount    = 4;
    D.Indices   = i;
    D.icount    = 6;

    D.Vertices[0].position = math::vector_t(0, 0);
    D.Vertices[1].position = math::vector_t(w, 0);
    D.Vertices[2].position = math::vector_t(w, h);
    D.Vertices[3].position = math::vector_t(0, h);

    D.Vertices[0].tc = math::vector_t(0.0, 1.0);
    D.Vertices[1].tc = math::vector_t(1.0, 1.0);
    D.Vertices[2].tc = math::vector_t(1.0, 0.0);
    D.Vertices[3].tc = math::vector_t(0.0, 0.0);

    D.Vertices[0].color =
    D.Vertices[1].color =
    D.Vertices[2].color =
    D.Vertices[3].color = color4f_t(1, 1, 1, 1);

    s_FullscreenQuad.Init();
    s_FullscreenQuad.AddData(D);
    s_FullscreenQuad.Offload();

    return true;
}
