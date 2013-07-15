// We inline all of the short functions so the compiler can optimize
// them if need be, since they are called quite often.

const gfx::CMaterial& CRenderer::GetDefaultMaterial()
{
    return *s_DefaultMaterial;
}

gfx::CEffect& CRenderer::GetDefaultEffect()
{
    return s_DefaultMaterial->GetEffect();
}

const gfxcore::CTexture& CRenderer::GetDefaultTexture()
{
    return s_DefaultMaterial->GetTexture();
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

bool CRenderer::BlendOperation(const BlendFunc& Func)
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

bool CRenderer::EnableTexture(const GLuint handle)
{
    GL(glBindTexture(GL_TEXTURE_2D, handle));
    return true;
}

bool CRenderer::DisableTexture()
{
    return EnableTexture(0);
}

CVertexArray& CRenderer::GetFullscreenVBO()
{
    return s_FullscreenQuad;
}
