#include "Zenderer/Graphics/Material.hpp"

using namespace zen;

using util::CLog;
using util::LogMode;
using gfx::CMaterial;

CMaterial::CMaterial(asset::CAssetManager& Assets) :
    m_Assets(Assets),
    m_Log(CLog::GetEngineLog()),
    m_Effect(EffectType::NO_EFFECT, Assets),
    m_Texture(gfxcore::CTexture::GetDefaultTexture())
{
    m_Effect.Init();
}

CMaterial::CMaterial(const CMaterial& Copy) :
    m_Assets(Copy.m_Assets),
    m_Log(CLog::GetEngineLog()),
    m_Effect(Copy.m_Effect.GetType(), m_Assets),
    m_Texture(gfxcore::CTexture::GetDefaultTexture())
{
    m_Effect.Init();

    // We don't want to reload the texture for no reason.
    if(&Copy.m_Texture != &m_Texture)
        m_Texture.LoadFromExisting(&Copy.m_Texture);
}

CMaterial::~CMaterial()
{
    this->Destroy();
}

bool CMaterial::LoadFromFile(const string_t& filename)
{
    this->Destroy();

    util::CINIParser Parser;
    Parser.LoadFromFile(filename);

    string_t
        vs = Parser.GetValue("vshader"),
        fs = Parser.GetValue("fshader"),
        tx = Parser.GetValue("texture");

    if(vs.empty() || fs.empty() || tx.empty())
    {
        m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                << m_Log.SetSystem("Material")
                << "Invalid .zfx file: Does not contain one of the following:"
                << CLog::endl << "\tVertex shader filename" << CLog::endl
                << "\tFragment shader filename" << CLog::endl
                << "\tTexture filename" << CLog::endl;

        return false;
    }

    m_Effect.SetType(gfx::EffectType::CUSTOM_EFFECT);
    m_Effect.Init();
    bool ret = m_Effect.LoadCustomEffect(vs, fs);
    ret = ret && m_Texture.LoadFromFile(tx);
    return ret;
}

bool CMaterial::LoadFromStream(std::ifstream& f,
                               const std::streampos& start,
                               const std::streampos& end)
{
    ZEN_ASSERT(f);

    f.seekg(start);
    util::CINIParser Parser;
    Parser.LoadFromStream(f, start, end);

    bool valid = false;
    if(Parser.Exists("texture"))
    {
        valid = this->LoadTextureFromFile(Parser.GetValue("texture"));
    }

    if(Parser.Exists("vshader") && Parser.Exists("fshader"))
    {
        valid = this->LoadEffect(gfx::EffectType::CUSTOM_EFFECT) &&
            m_Effect.LoadCustomEffect(Parser.GetValue("vshader"),
                                      Parser.GetValue("fshader"));
    }

    f.seekg(start);
    return valid && f;
}

bool CMaterial::LoadTextureFromFile(const string_t& filename)
{
    gfxcore::CTexture* tmp = m_Assets.Create<gfxcore::CTexture>(filename);
    if(tmp == nullptr) return false;
    m_Texture.LoadFromExisting(tmp);
    m_Assets.Delete(tmp);
    return true;
}

bool CMaterial::LoadTextureFromHandle(const GLuint handle)
{
    return m_Texture.LoadFromExisting(handle);
}

bool CMaterial::LoadEffect(const gfx::EffectType Type)
{
    if(m_Effect.GetType() != Type)
    {
        m_Effect.Destroy();
        m_Effect.SetType(Type);
        return m_Effect.Init();
    }

    return true;
}

bool CMaterial::Attach(gfx::CEffect& E, gfxcore::CTexture& T)
{
    m_Effect = E;
    return m_Texture.LoadFromExisting(&T);
}

bool CMaterial::Enable() const
{
    return m_Effect.Enable() && m_Texture.Bind();
}

bool CMaterial::EnableEffect() const
{
    return m_Effect.Enable();
}

bool CMaterial::EnableTexture() const
{
    return m_Texture.Bind();
}

bool CMaterial::Disable() const
{
    return m_Effect.Disable() && m_Texture.Unbind();
}

bool CMaterial::DisableEffect() const
{
    return m_Effect.Disable();
}

bool CMaterial::DisableTexture() const
{
    return m_Texture.Unbind();
}

gfx::CEffect& CMaterial::GetEffect()
{
    return m_Effect;
}

gfxcore::CTexture& CMaterial::GetTexture() const
{
    return m_Texture;
}

void CMaterial::Destroy()
{
    m_Effect.Destroy();
}
