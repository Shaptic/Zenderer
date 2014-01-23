#include "Zenderer/Graphics/Light.hpp"

using namespace zen;
using gfx::zLight;

math::vector_t  zLight::s_DefaultAttenuation(0.05, 0.01, 0.0),
                zLight::s_DefaultPosition(0, 0, 0);

color3f_t       zLight::s_DefaultColor(1.0, 1.0, 1.0);

real_t          zLight::s_DefaultBrightness = 1.0,
                zLight::s_DefaultMaxAngle(45.0),
                zLight::s_DefaultMinAngle(-45.0);

zLight::zLight(asset::zAssetManager& m_Assets,
               const gfx::LightType& Type,
               const uint16_t window_h /*= 800*/) :
    m_Shader(m_Assets), m_type(Type),
    m_loccol(-1), m_locbrt(-1), m_locpos(-1), m_locatt(-1),
    m_loctmx(-1), m_loctmn(-1), m_height(window_h)
{
}

zLight::~zLight()
{
    m_Shader.Destroy();
}

bool zLight::Init()
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
        m_locatt = m_Shader.GetUniformLocation("light_att");

        GLint mvloc   = m_Shader.GetUniformLocation("mv"),
              projloc = m_Shader.GetUniformLocation("proj"),
              scrloc  = m_Shader.GetUniformLocation("scr_height");

        if(m_loccol == -1 || m_locbrt == -1 || mvloc == -1 || projloc == -1)
        {
            m_Shader.Destroy();
            return false;
        }
        else
        {
            // Set up the values we only need a single time.
            if(!m_Shader.Bind()) return false;

            GL(glUniformMatrix4fv(mvloc, 1, GL_TRUE,
                math::mat4_t().AsPtr()));

            GL(glUniformMatrix4fv(projloc, 1, GL_TRUE,
                gfxcore::zRenderer::GetProjectionMatrix().AsPtr()));

            // This isn't in all lights but we set it if it was found.
            GL(glUniform1i(scrloc, m_height));

            // Set default values, even if they don't exist it doesn't matter.
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

void zLight::Adjust(const real_t dx, const real_t dy)
{
    this->SetPosition(m_Position.x + dx, m_Position.y + dy);
}

bool zLight::Enable() const
{
    return m_Shader.Bind();
}

bool zLight::Disable() const
{
    return m_Shader.Unbind();
}

bool zLight::SetBrightness(const real_t brightness)
{
    m_brt = brightness;
    GL(glUniform1f(m_locbrt, m_brt));
    return true;
}

bool zLight::SetColor(const real_t r, const real_t g, const real_t b)
{
    m_Color = color3f_t(r, g, b);
    GL(glUniform3f(m_loccol, r, g, b));
    return true;
}

bool zLight::SetColor(const color3f_t& Color)
{
    return this->SetColor(Color.r, Color.g, Color.b);
}

bool zLight::SetAttenuation(const real_t c, const real_t l, const real_t q)
{
    m_Att = math::vector_t(c, l, q);
    GL(glUniform3f(m_locatt, c, l, q));
    return true;
}

bool zLight::SetAttenuation(const math::vector_t& Att)
{
    return this->SetAttenuation(Att.x, Att.y, Att.z);
}

/// @todo   Figure out why the 200px offset is necessary.
bool zLight::SetPosition(const real_t x, const real_t y)
{
    m_Position = math::vector_t(x, y);
    GL(glUniform2f(m_locpos, x, y));
    return true;
}

bool zLight::SetPosition(const math::vector_t& Pos)
{
    return this->SetPosition(Pos.x, Pos.y);
}

bool zLight::SetMaximumAngle(const real_t degrees)
{
    m_Max = math::vector_t(1, 0, 0);
    m_Max.Rotate(math::rad(degrees));

    GL(glUniform2f(m_loctmx, m_Max.x, m_Max.y));
    return true;
}

bool zLight::SetMinimumAngle(const real_t degrees)
{
    m_Min = math::vector_t(1, 0, 0);
    m_Min.Rotate(math::rad(degrees));

    GL(glUniform2f(m_loctmn, m_Min.x, m_Min.y));
    return true;
}

void zLight::SetType(const gfx::LightType& Type)
{
    m_type = Type;
}

zen::real_t zLight::GetBrightness() const
{
    return m_brt;
}

const color3f_t& zLight::GetColor() const
{
    return m_Color;
}

const math::vector_t& zLight::GetPosition() const
{
    return m_Position;
}

gfx::LightType zLight::GetType() const
{
    return m_type;
}
