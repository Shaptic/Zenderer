#include "Zenderer/Graphics/Material.hpp"

using namespace zen;

using util::zLog;
using util::LogMode;
using gfx::zMaterial;

zMaterial::zMaterial(asset::zAssetManager& Assets) :
    m_Assets(Assets), m_Log(zLog::GetEngineLog()),
    mp_Texture(&gfxcore::zTexture::GetDefaultTexture()),
    m_Effect(EffectType::NO_EFFECT, Assets), m_ID(0)
{
    m_Effect.Init();
    this->SetID();
}

zMaterial::zMaterial(const zMaterial& Copy) :
    m_Assets(Copy.m_Assets),
    m_Log(zLog::GetEngineLog()),
    mp_Texture(&gfxcore::zTexture::GetDefaultTexture()),
    m_Effect(Copy.m_Effect.GetType(), m_Assets)
{
    m_Effect.Init();

    // We don't want to reload the texture for no reason.
    if(Copy.mp_Texture != mp_Texture)
        mp_Texture->LoadFromExisting(Copy.mp_Texture);

    this->SetID();
}

zMaterial::~zMaterial()
{
    this->Destroy();
}

bool zMaterial::LoadFromFile(const string_t& filename)
{
    this->Destroy();

    util::zParser Parser;
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
                << zLog::endl << "\tVertex shader filename" << zLog::endl
                << "\tFragment shader filename" << zLog::endl
                << "\tTexture filename" << zLog::endl;

        return false;
    }

    m_Effect.SetType(gfx::EffectType::CUSTOM_EFFECT);
    m_Effect.Init();
    bool ret = m_Effect.LoadCustomEffect(vs, fs);
    ret = ret && mp_Texture->LoadFromFile(tx);
    this->SetID();
    return ret;
}

bool zMaterial::LoadFromStream(std::ifstream& f,
                               const std::streampos& start,
                               const std::streampos& end)
{
    ZEN_ASSERT(f);

    f.seekg(start);
    util::zParser Parser;
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

bool zMaterial::LoadTexture(const gfxcore::zTexture& Texture)
{
    gfxcore::zTexture* tmp = m_Assets.Create<gfxcore::zTexture>(
        Texture.GetFilename(), Texture.GetOwner());

    if(tmp == nullptr)
    {
        // This is a special case where we want to be assigned to the 1x1
        // white texture. We can safely store it internally since it will
        // not be modified, because other methods make sure we are not the
        // default before changing it.
        if(&Texture == &Texture.GetDefaultTexture())
        {
            mp_Texture = const_cast<gfxcore::zTexture*>(&Texture);
            return true;
        }

        m_Assets.Delete(tmp);
        return false;
    }

    mp_Texture = tmp;
    this->SetID();
    return true;
}

bool zMaterial::LoadTextureFromFile(const string_t& filename)
{
    gfxcore::zTexture* tmp = m_Assets.Create<gfxcore::zTexture>(filename);
    mp_Texture = tmp;
    if(mp_Texture == nullptr)
    {
        mp_Texture = &gfxcore::zTexture::GetDefaultTexture();
        this->SetID();
        return false;
    }

    return true;
}

bool zMaterial::LoadTextureFromHandle(const GLuint handle)
{
    bool ret = mp_Texture->LoadFromExisting(handle);
    this->SetID();
    return ret;
}

bool zMaterial::LoadEffect(const gfx::EffectType Type)
{
    bool ret = true;
    if(m_Effect.GetType() != Type)
    {
        m_Effect.Destroy();
        m_Effect.SetType(Type);
        ret = m_Effect.Init();
        this->SetID();
    }

    return ret;
}

bool zMaterial::Attach(gfx::zEffect& E, gfxcore::zTexture& T)
{
    m_Effect = E;
    bool ret = mp_Texture->LoadFromExisting(&T);
    this->SetID();
    return ret;
}

bool zMaterial::Enable() const
{
    return m_Effect.Enable() && mp_Texture->Bind();
}

bool zMaterial::EnableEffect() const
{
    return m_Effect.Enable();
}

bool zMaterial::EnableTexture() const
{
    return mp_Texture->Bind();
}

bool zMaterial::Disable() const
{
    return m_Effect.Disable() && mp_Texture->Unbind();
}

bool zMaterial::DisableEffect() const
{
    return m_Effect.Disable();
}

bool zMaterial::DisableTexture() const
{
    return mp_Texture->Unbind();
}

gfx::zEffect& zMaterial::GetEffect()
{
    return m_Effect;
}

const gfx::zEffect& zMaterial::GetEffect() const
{
    return m_Effect;
}

const gfxcore::zTexture& zMaterial::GetTexture() const
{
    return *mp_Texture;
}

void zMaterial::Destroy()
{
    m_Effect.Destroy();
    if(mp_Texture != &gfxcore::zTexture::GetDefaultTexture())
        m_Assets.Delete(mp_Texture);
    this->SetID();
}

void zMaterial::SetID()
{
    m_ID = (mp_Texture->GetID() << 10) & m_Effect.GetID();
    ZEN_ASSERT(m_ID == 0);
}
