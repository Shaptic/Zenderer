#include "Zenderer/Graphics/Effect.hpp"

using namespace zen;

using util::CLog;
using util::LogMode;

using gfx::CEffect;
using gfx::EffectType;

CEffect::CEffect(const EffectType Type, asset::CAssetManager& Assets) :
    CGLSubsystem("ShaderSet"),
    m_Log(CLog::GetEngineLog()),
    m_Shader(Assets), m_type(Type)
{
}

CEffect::~CEffect()
{
    this->Destroy();
}

bool CEffect::Init()
{
    if(m_init) return true;

    // m_type is guaranteed to be valid, except for counter.
    if(m_type == EffectType::ZEN_EFFECT_COUNT)
    {
        m_Log << m_Log.SetMode(LogMode::ZEN_ERROR) << m_Log.SetSystem("Effect")
              << "ZEN_EFFECT_COUNT is not a valid effect." << CLog::endl;
        return (m_init = false);
    }

#ifdef ZEN_DEBUG_BUILD
    // All effects currently use the default vertex shader.
    m_Shader.LoadVertexShaderFromFile(ZENDERER_SHADER_PATH"Default.vs");

    if(m_type == EffectType::NO_EFFECT)
        m_init = m_Shader.LoadFragmentShaderFromFile(
            ZENDERER_SHADER_PATH"Default.fs");

    else if(m_type == EffectType::GAUSSIAN_BLUR_H)
        m_init = m_Shader.LoadFragmentShaderFromFile(
            ZENDERER_SHADER_PATH"GuassianBlurH.fs");

    else if(m_type == EffectType::GAUSSIAN_BLUR_V)
        m_init = m_Shader.LoadFragmentShaderFromFile(
            ZENDERER_SHADER_PATH"GuassianBlurV.fs");

    else if(m_type == EffectType::GRAYSCALE)
        m_init = m_Shader.LoadFragmentShaderFromFile(
            ZENDERER_SHADER_PATH"TTFRender.fs");

    else if(m_type == EffectType::RIPPLE)
        m_init = m_Shader.LoadFragmentShaderFromFile(
            ZENDERER_SHADER_PATH"Ripple.fs");
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
        m_init = m_Shader.LoadFragmentShaderFromStr(gfxcore::TTFRENDER_FS);

    else if(m_type == EffectType::RIPPLE)
        m_init = m_Shader.LoadFragmentShaderFromFile(gfxcore::RIPPLE_FS);

#endif // ZEN_DEBUG_BUILD

    else if(m_type == EffectType::CUSTOM_EFFECT)
    {
        // Custom effects are only loaded from files via `CMaterial` class.
        m_Log   << m_Log.SetMode(LogMode::ZEN_FATAL) << m_Log.SetSystem("Effect")
                << "Custom effects can only be loaded from .zfx files via the "
                <<  "zen::gfx::CMaterial object." << CLog::endl;

        return(m_init = false);
    }

    else
    {
        // Fatal error because this shouldn't be possible.
        m_Log   << m_Log.SetMode(LogMode::ZEN_FATAL) << m_Log.SetSystem("Effect")
                << "Invalid effect type: " << static_cast<int16_t>(m_type)
                << "." << CLog::endl;

        return (m_init = false);
    }

    return (m_init = m_Shader.CreateShaderObject());
}

bool CEffect::Destroy()
{
    // We keep the type to Init() again.
    m_Shader.Destroy();
    return m_Shader.GetShaderObject() == 0;
}

bool CEffect::SetParameter(const string_t& name,
                           const real_t* pValues,
                           const size_t count)
{
    ZEN_ASSERT(!name.empty());
    ZEN_ASSERT(pValues != nullptr);
    ZEN_ASSERT(count > 0);

    if(!this->Init()) return false;

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
                << " real values." << CLog::endl;
        return false;
    }

    return true;
}

bool CEffect::SetParameter(const string_t& name,
                           const int* pValues,
                           const size_t count)
{
    ZEN_ASSERT(!name.empty());
    ZEN_ASSERT(pValues != nullptr);
    ZEN_ASSERT(count > 0);

    if(!this->Init()) return false;

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
                << " integer values." << CLog::endl;
        return false;
    }

    return true;
}

bool CEffect::SetParameter(const string_t& name,
                           const math::matrix4x4_t& Matrix)
{
    ZEN_ASSERT(!name.empty());

    if(!this->Init()) return false;

    GLint loc = m_Shader.GetUniformLocation(name);
    GL(glUniformMatrix4fv(loc, 1, GL_TRUE, Matrix.GetPointer()));
    return (loc != -1);
}

bool CEffect::LoadCustomEffect(const string_t& vs, const string_t& fs)
{
    ZEN_ASSERT(m_type == EffectType::CUSTOM_EFFECT);

    if(m_init) this->Destroy();
    m_init = m_Shader.LoadFromFile(vs, fs) && m_Shader.CreateShaderObject();
    return m_init;
}
