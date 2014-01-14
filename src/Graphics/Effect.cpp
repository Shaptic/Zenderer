#include "Zenderer/Graphics/Effect.hpp"

using namespace zen;

using util::zLog;
using util::LogMode;

using gfx::zEffect;
using gfx::EffectType;

zEffect::zEffect(const EffectType Type, asset::zAssetManager& Assets) :
    zGLSubsystem("ShaderSet"),
    m_Log(zLog::GetEngineLog()),
    m_Shader(Assets), m_type(Type),
    m_mvloc(-1), m_projloc(-1)
{
}

zEffect::~zEffect()
{
    this->Destroy();
}

bool zEffect::Init()
{
    if(m_init) return true;

    // m_type is guaranteed to be valid, except for counter.
    if(m_type == EffectType::ZEN_EFFECT_COUNT)
    {
        m_Log << m_Log.SetMode(LogMode::ZEN_ERROR) << m_Log.SetSystem("Effect")
              << "ZEN_EFFECT_COUNT is not a valid effect." << zLog::endl;
        return false;
    }

//#ifdef ZEN_DEBUG_BUILD
    // All effects currently use the default vertex shader.
    if (!m_Shader.LoadVertexShaderFromFile(ZENDERER_SHADER_PATH"Default.vs"))
        return false;

    if(m_type == EffectType::NO_EFFECT)
        m_init = m_Shader.LoadFragmentShaderFromFile(
            ZENDERER_SHADER_PATH"Default.fs");

    else if(m_type == EffectType::GAUSSIAN_BLUR_H)
        m_init = m_Shader.LoadFragmentShaderFromFile(
            ZENDERER_SHADER_PATH"GaussianBlurH.fs");

    else if(m_type == EffectType::GAUSSIAN_BLUR_V)
        m_init = m_Shader.LoadFragmentShaderFromFile(
            ZENDERER_SHADER_PATH"GaussianBlurV.fs");

    else if(m_type == EffectType::GRAYSCALE)
        m_init = m_Shader.LoadFragmentShaderFromFile(
            ZENDERER_SHADER_PATH"Grayscale.fs");

    else if(m_type == EffectType::SPRITESHEET)
        m_init = m_Shader.LoadFragmentShaderFromFile(
            ZENDERER_SHADER_PATH"Spritesheet.fs");

    else if(m_type == EffectType::ZFONT)
        m_init = m_Shader.LoadFragmentShaderFromFile(
            ZENDERER_SHADER_PATH"zFont.fs");

    else if(m_type == EffectType::RIPPLE)
        m_init = m_Shader.LoadFragmentShaderFromFile(
            ZENDERER_SHADER_PATH"Ripple.fs");

    else if(m_type == EffectType::SHADOW_MAP_GENERATOR)
        m_init = m_Shader.LoadFragmentShaderFromFile(
            ZENDERER_SHADER_PATH"ShadowShader.fs");

    else if (m_type == EffectType::SHADED_LIGHT_RENDERER)
        m_init = m_Shader.LoadFragmentShaderFromFile(
            ZENDERER_SHADER_PATH"ShadowLight.fs");

    /*
#else
    // All effects currently use the default vertex shader.
    m_Shader.LoadVertexShaderFromStr(gfxcore::DEFAULT_VS);

    if(m_type == EffectType::NO_EFFECT)
        m_init = m_Shader.LoadFragmentShaderFromStr(gfxcore::DEFAULT_FS);

    else if(m_type == EffectType::GAUSSIAN_BLUR_H)
        m_init = m_Shader.LoadFragmentShaderFromStr(gfxcore::GAUSSIANBLURH_FS);

    else if(m_type == EffectType::GAUSSIAN_BLUR_V)
        m_init = m_Shader.LoadFragmentShaderFromStr(gfxcore::GAUSSIANBLURV_FS);

    else if(m_type == EffectType::GRAYSCALE)
        m_init = m_Shader.LoadFragmentShaderFromStr(gfxcore::GRAYSCALE_FS);

    else if(m_type == EffectType::SPRITESHEET)
        m_init = m_Shader.LoadFragmentShaderFromStr(gfxcore::SPRITESHEET_FS);

    else if(m_type == EffectType::RIPPLE)
        m_init = m_Shader.LoadFragmentShaderFromStr(gfxcore::RIPPLE_FS);

#endif // ZEN_DEBUG_BUILD
    */

    else if(m_type == EffectType::CUSTOM_EFFECT)
    {
        // Custom effects are only loaded from files via `zMaterial` class.
        m_Log   << m_Log.SetMode(LogMode::ZEN_FATAL) << m_Log.SetSystem("Effect")
                << "Custom effects can only be loaded from .zfx files via the "
                <<  "zen::gfx::zMaterial object." << zLog::endl;

        return false;
    }

    else
    {
        // Fatal error because this shouldn't be possible.
        m_Log   << m_Log.SetMode(LogMode::ZEN_FATAL) << m_Log.SetSystem("Effect")
                << "Invalid effect type: " << static_cast<int16_t>(m_type)
                << "." << zLog::endl;

        return false;
    }

    if(!m_Shader.CreateShaderObject())
        return false;

    m_mvloc = m_Shader.GetUniformLocation("mv");
    m_projloc = m_Shader.GetUniformLocation("proj");
    return (m_init = (m_mvloc != m_projloc));
}

bool zEffect::Destroy()
{
    // We keep the type to Init() again.
    m_Shader.Destroy();
    return !(m_init = (m_Shader.GetShaderObject() != 0));
}

bool zEffect::SetParameter(const string_t& name,
                           const real_t* pValues,
                           const size_t count)
{
    ZEN_ASSERT(!name.empty());
    ZEN_ASSERT(pValues != nullptr);
    ZEN_ASSERT(count > 0);

    if(!m_init) return false;

    GLint loc = m_Shader.GetUniformLocation(name);
    if(loc == -1) return false;

    if(count == 1)      { GL(glUniform1f(loc, pValues[0]));  }
    else if(count == 2) { GL(glUniform2fv(loc, 1, pValues)); }
    else if(count == 3) { GL(glUniform3fv(loc, 1, pValues)); }
    else if(count == 4) { GL(glUniform4fv(loc, 1, pValues)); }
    else
    {
        m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                << m_Log.SetSystem("Effect")
                << "No appropriate parameter switch found for " << count
                << " real values." << zLog::endl;
        return false;
    }

    return true;
}

bool zEffect::SetParameter(const string_t& name,
                           const int* pValues,
                           const size_t count)
{
    ZEN_ASSERT(!name.empty());
    ZEN_ASSERT(pValues != nullptr);
    ZEN_ASSERT(count > 0);

    if(!m_init) return false;

    GLint loc = m_Shader.GetUniformLocation(name);
    if(loc == -1) return false;

    if(count == 1)      { GL(glUniform1i(loc, pValues[0]));  }
    else if(count == 2) { GL(glUniform2iv(loc, 1, pValues)); }
    else if(count == 3) { GL(glUniform3iv(loc, 1, pValues)); }
    else if(count == 4) { GL(glUniform4iv(loc, 1, pValues)); }
    else
    {
        m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                << m_Log.SetSystem("Effect")
                << "No appropriate parameter switch found for " << count
                << " integer values." << zLog::endl;
        return false;
    }

    return true;
}

bool zEffect::SetParameter(const string_t& name,
                           const math::matrix4x4_t& Matrix)
{
    ZEN_ASSERT(!name.empty());
    ZEN_ASSERTM(name != "mv" && name != "proj",
        "deprecated functionality, use Set[Projection|Model]Matrix instead");

    if(!m_init) return false;
    GLint loc = m_Shader.GetUniformLocation(name);
    return this->SetMatrix(loc, Matrix);
}

bool zEffect::SetProjectionMatrix(const math::matrix4x4_t& Projection) const
{
    return this->SetMatrix(m_projloc, Projection);
}

bool zEffect::SetModelMatrix(const math::matrix4x4_t& ModelView) const
{
    return this->SetMatrix(m_mvloc, ModelView);
}

bool zEffect::LoadCustomEffect(const string_t& vs, const string_t& fs)
{
    ZEN_ASSERT(m_type == EffectType::CUSTOM_EFFECT);

    if(m_init) this->Destroy();
    m_init = m_Shader.LoadFromFile(vs, fs) && m_Shader.CreateShaderObject();
    return m_init;
}

bool zEffect::SetMatrix(GLint loc, const math::matrix4x4_t& matrix) const
{
    if(loc == -1 || !m_init) return false;
    GL(glUniformMatrix4fv(loc, 1, GL_TRUE, matrix.GetPointer()));
    return true;
}
