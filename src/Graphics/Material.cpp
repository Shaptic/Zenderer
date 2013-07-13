#include "Zenderer/Graphics/Material.hpp"

using namespace zen;

using util::CLog;
using util::LogMode;
using gfx::CMaterial;

CMaterial::CMaterial(asset::CAssetManager& Assets) :
    m_Assets(Assets),
    mp_Texture(&gfxcore::CTexture::GetDefaultTexture()),
    m_Log(CLog::GetEngineLog()),
    m_Effect(EffectType::NO_EFFECT, Assets)
{
    m_Effect.Init();
}

CMaterial::CMaterial(const CMaterial& Copy) :
    m_Assets(Copy.m_Assets),
    m_Log(CLog::GetEngineLog()),
    m_Effect(Copy.m_Effect.GetType(), m_Assets),
    mp_Texture(&gfxcore::CTexture::GetDefaultTexture())
{
    m_Effect.Init();

    // We don't want to reload the texture for no reason.
    if(Copy.mp_Texture != mp_Texture)
        mp_Texture->LoadFromExisting(Copy.mp_Texture);
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
    ret = ret && mp_Texture->LoadFromFile(tx);
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

bool CMaterial::LoadTexture(const gfxcore::CTexture& Texture)
{
    gfxcore::CTexture* tmp = m_Assets.Create<gfxcore::CTexture>(
        Texture.GetFilename(), Texture.GetOwner());

    if(tmp == nullptr)
    {
        // This is a special case where we want to be assigned to the 1x1
        // white texture. We can safely store it internally since it will
        // not be modified, because other methods make sure we are not the
        // default before changing it.
        if(&Texture == &Texture.GetDefaultTexture())
        {
            mp_Texture = const_cast<gfxcore::CTexture*>(&Texture);
            return true;
        }

        m_Assets.Delete(tmp);
        return false;
    }

    mp_Texture = tmp;
    return true;
}

bool CMaterial::LoadTextureFromFile(const string_t& filename)
{
    gfxcore::CTexture* tmp = m_Assets.Create<gfxcore::CTexture>(filename);
    return ((mp_Texture = tmp) != nullptr);
}

bool CMaterial::LoadTextureFromHandle(const GLuint handle)
{
    return mp_Texture->LoadFromExisting(handle);
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
    return mp_Texture->LoadFromExisting(&T);
}

bool CMaterial::Enable() const
{
    return m_Effect.Enable() && mp_Texture->Bind();
}

bool CMaterial::EnableEffect() const
{
    return m_Effect.Enable();
}

bool CMaterial::EnableTexture() const
{
    return mp_Texture->Bind();
}

bool CMaterial::Disable() const
{
    return m_Effect.Disable() && mp_Texture->Unbind();
}

bool CMaterial::DisableEffect() const
{
    return m_Effect.Disable();
}

bool CMaterial::DisableTexture() const
{
    return mp_Texture->Unbind();
}

gfx::CEffect& CMaterial::GetEffect()
{
    return m_Effect;
}

const gfxcore::CTexture& CMaterial::GetTexture() const
{
    return *mp_Texture;
}

void CMaterial::Destroy()
{
    m_Effect.Destroy();
    if(mp_Texture != &gfxcore::CTexture::GetDefaultTexture())
        m_Assets.Delete(mp_Texture);
}
