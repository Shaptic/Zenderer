// We inline all of the short functions so the compiler can optimize
// them if need be, since they are called quite often.

gfx::CEffect& CRenderer::GetDefaultEffect()
{
    return s_DefaultShader;
}

const math::matrix4x4_t& CRenderer::GetProjectionMatrix()
{
    return s_ProjMatrix;
}

bool CRenderer::ResetMaterialState()
{
    GL(glBindTexture(GL_TEXTURE_2D, 0));
    GL(glUseProgram(0));
    return true;
}

bool CRenderer::EnableBlending()
{
    GL(glEnable(GL_BLEND));
    GL(glDisable(GL_DEPTH_TEST));
    return EnableSTDBlendFunc();
}

bool CRenderer::DisableBlending()
{
    GL(glDisable(GL_BLEND));
    GL(glEnable(GL_DEPTH_TEST));
    return true;
}

bool CRenderer::EnableTexture(const GLuint handle)
{
    GL(glBindTexture(GL_TEXTURE_2D, handle));
    return true;
}

bool CRenderer::DisableTexture()
{
    return EnableTexture(0);
}

bool CRenderer::EnableSTDBlendFunc()
{
    GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    return true;
}

const CVertexArray& CRenderer::GetFullscreenVBO()
{
    return s_FullscreenQuad;
}