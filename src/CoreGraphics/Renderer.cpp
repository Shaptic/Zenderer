#include "Zenderer/CoreGraphics/Renderer.hpp"

using namespace zen;
using gfxcore::CRenderer;

gfx::CEffect CRenderer::s_DefaultShader(gfx::EffectType::NO_EFFECT);
math::matrix4x4_t CRenderer::s_ProjMatrix;

