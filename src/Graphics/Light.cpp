#include "Zenderer/Graphics/Light.hpp"

using namespace zen;
using gfx::CLight;

math::vector_t  CLight::s_DefaultAttenuation,
                CLight::s_DefaultPosition(0, 0, 0);

color3f_t       CLight::s_DefaultColor(1.0, 1.0, 1.0);

real_t          CLight::s_DefaultBrightness = 1.0,
                CLight::s_DefaultMaxAngle(180.0),
                CLight::s_DefaultMinAngle(0.0);

CLight::CLight(asset::CAssetManager& m_Assets,
               const gfx::LightType& Type,
               const uint16_t window_h /*= 800*/) :
    m_type(Type), m_Shader(m_Assets), m_height(window_h),

    m_loccol(-1), m_locbrt(-1), m_locpos(-1), m_locatt(-1),
    m_loctmx(-1), m_loctmn(-1)
{

}

CLight::~CLight()
{
    m_Shader.Destroy();
}

bool CLight::Init()
{
    bool state = false;

    state = m_Shader.LoadVertexShaderFromFile(
        ZENDERER_SHADER_PATH"Default.vs");

    switch(m_type)
    {
    case LightType::ZEN_AMBIENT:
        state = state && m_Shader.LoadFragmentShaderFromFile(
            ZENDERER_SHADER_PATH"AmbientLight.fs");
        break;

    case LightType::ZEN_POINT:
        state = state && m_Shader.LoadFragmentShaderFromFile(
            ZENDERER_SHADER_PATH"PointLight.fs");
        break;

    case LightType::ZEN_SPOTLIGHT:
        state = state && m_Shader.LoadFragmentShaderFromFile(
            ZENDERER_SHADER_PATH"SpotLight.fs");
        break;

    default: break;
    }

    state = state && m_Shader.CreateShaderObject();

    if(state)
    {
        m_loccol = m_Shader.GetUniformLocation("light_col");
        m_locbrt = m_Shader.GetUniformLocation("light_brt");
        m_locpos = m_Shader.GetUniformLocation("light_pos");
        m_loctmx = m_Shader.GetUniformLocation("light_max");
        m_loctmn = m_Shader.GetUniformLocation("light_min");

        GLint mvloc = m_Shader.GetUniformLocation("mv"),
            projloc = m_Shader.GetUniformLocation("proj"),
            scrloc  = m_Shader.GetUniformLocation("scr_height");

        if(m_loccol == -1   || m_locbrt == -1 || mvloc == -1 || projloc == -1)
        {
            return false;
        }
        else
        {
            // Set up the values we only need a single time.
            if(!m_Shader.Bind()) return false;

            GL(glUniformMatrix4fv(mvloc, 1, GL_TRUE,
                math::matrix4x4_t
                    ::GetIdentityMatrix().GetPointer()));

            GL(glUniformMatrix4fv(projloc, 1, GL_TRUE,
                gfxcore::CRenderer
                       ::GetProjectionMatrix().GetPointer()));

            // This isn't in all lights but we set it if it was found.
            GL(glUniform1i(scrloc, m_height));

            // Set default values.
            this->SetAttenuation(s_DefaultAttenuation);
            this->SetPosition(s_DefaultPosition);
            this->SetColor(s_DefaultColor);
            this->SetBrightness(s_DefaultBrightness);
            this->SetMinimumAngle(s_DefaultMinAngle);
            this->SetMaximumAngle(s_DefaultMaxAngle);

            return m_Shader.Unbind();
        }
    }

    return state;
}

bool CLight::Enable()
{
    return m_Shader.Bind();
}

bool CLight::Disable()
{
    return m_Shader.Unbind();
}

void CLight::SetBrightness(const real_t brightness)
{
    m_brt = brightness;
    GL(glUniform1f(m_locbrt, m_brt));
}

void CLight::SetColor(const real_t r, const real_t g, const real_t b)
{
    m_Color = color3f_t(r, g, b); 
    GL(glUniform3f(m_loccol, r, g, b));
}

void CLight::SetColor(const color3f_t& Color)
{
    this->SetColor(Color.r, Color.g, Color.b);
}

void CLight::SetAttenuation(const real_t c, const real_t l, const real_t q)
{
    m_Att = math::vector_t(c, l, q);
    GL(glUniform3f(m_locatt, c, l, q));
}

void CLight::SetAttenuation(const math::vector_t& Att)
{
    this->SetAttenuation(Att.x, Att.y, Att.z);
}

void CLight::SetPosition(const real_t x, const real_t y)
{
    m_Position = math::vector_t(x, y);
    GL(glUniform2f(m_locpos, x, y));
}

void CLight::SetPosition(const math::vector_t& Pos)
{
    this->SetPosition(Pos.x, Pos.y);
}

void CLight::SetMaximumAngle(const real_t degrees)
{
    m_Max = math::vector_t(1, 0);
    m_Max.Rotate(math::rad(degrees));

    GL(glUniform2f(m_loctmx, m_Max.x, m_Max.y));
}

void CLight::SetMinimumAngle(const real_t degrees)
{
    m_Min = math::vector_t(1, 0);
    m_Min.Rotate(math::rad(degrees));

    GL(glUniform2f(m_loctmn, m_Min.x, m_Min.y));
}