// We inline all of the short functions so the compiler can optimize
// them if need be, since they are called quite often.

const gfx::zMaterial& zRenderer::GetDefaultMaterial()
{
    return *s_DefaultMaterial;
}

const gfx::zEffect& zRenderer::GetDefaultEffect()
{
    return s_DefaultMaterial->GetEffect();
}

const gfxcore::zTexture& zRenderer::GetDefaultTexture()
{
    return s_DefaultMaterial->GetTexture();
}

const math::matrix4x4_t& zRenderer::GetProjectionMatrix()
{
    return s_ProjMatrix;
}

bool zRenderer::ResetMaterialState()
{
    GL(glBindTexture(GL_TEXTURE_2D, 0));
    GL(glUseProgram(0));
    return true;
}

bool zRenderer::BlendOperation(const BlendFunc& Func)
{
    if(s_LastBlend == Func) return true;

    switch(Func)
    {
    case BlendFunc::DISABLE_BLEND:
        if(!s_blend) break;
        GL(glDisable(GL_BLEND));
        GL(glEnable(GL_DEPTH_TEST));
        s_blend = false;
        break;

    case BlendFunc::ENABLE_BLEND:
        if(s_blend) break;
        GL(glEnable(GL_BLEND));
        GL(glDisable(GL_DEPTH_TEST));
        s_blend = true;
        break;

    case BlendFunc::STANDARD_BLEND:
        BlendOperation(BlendFunc::ENABLE_BLEND);
        GL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        break;

    case BlendFunc::ADDITIVE_BLEND:
        BlendOperation(BlendFunc::ENABLE_BLEND);
        GL(glBlendFunc(GL_ONE, GL_ONE));
        break;

    case BlendFunc::IS_ENABLED:
    {
        GLboolean status = GL_FALSE;
        GL(status = glIsEnabled(GL_BLEND));
        return (s_blend = (status == GL_TRUE));
    }
    }

    s_LastBlend = Func;
    return true;
}

bool zRenderer::EnableTexture(const GLuint handle)
{
    GL(glBindTexture(GL_TEXTURE_2D, handle));
    return true;
}

bool zRenderer::ToggleWireframe()
{
    s_wf = !s_wf;
    if(s_wf)
    {
        GL(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }
    else
    {
        GL(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }

    return s_wf;
}

bool zRenderer::DisableTexture()
{
    return EnableTexture(0);
}

zVertexArray& zRenderer::GetFullscreenVBO()
{
    return s_FullscreenQuad;
}

void zRenderer::SetProjectionMatrix(const math::matrix4x4_t& Proj)
{
    s_ProjMatrix = Proj;
}
