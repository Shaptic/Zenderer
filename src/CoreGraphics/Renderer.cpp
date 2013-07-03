#include "Zenderer/CoreGraphics/Renderer.hpp"

using namespace zen;
using gfxcore::CRenderer;

gfx::CEffect            CRenderer::s_DefaultShader(gfx::EffectType::NO_EFFECT);
gfxcore::CVertexArray   CRenderer::s_FullscreenQuad(GL_STATIC_DRAW);
math::matrix4x4_t       CRenderer::s_ProjMatrix;

bool CRenderer::LoadVAO(const uint16_t w, const uint16_t h)
{
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
}