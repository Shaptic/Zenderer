#include "Zenderer/Graphics/Material.hpp"

using namespace zen;

using util::CLog;
using util::LogMode;
using gfx::CMaterial;

CMaterial::CMaterial(asset::CAssetManager& Assets) :
    m_Assets(Assets), m_Effect(EffectType::NO_EFFECT),
    mp_Texture(nullptr), 
    m_Log(CLog::GetEngineLog())
{
    mp_Texture = &CRenderer::GetDefaultTexture();
}

CMaterial::CMaterial(const CMaterial& Copy) : 
    m_Assets(Copy.m_Assets),
    m_Effect(Copy.m_Effect.GetType()),
    mp_Texture(nullptr),
    m_Log(CLog::GetEngineLog())
{
    m_Effect.Init();
    mp_Texture = m_Assets.Create<gfxcore::CTexture>();
    
    // We don't want to reload the texture for no reason.
    if(Copy.mp_Texture != mp_Texture)
        mp_Texture.LoadFromExisting(Copy.mp_Texture);
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

bool CMaterial::LoadTextureFromFile(const string_t& filename)
{
    mp_Texture = m_Assets.Create<gfxcore::CTexture>(filename);
    return (mp_Texture != nullptr);
}

bool CMaterial::LoadTextureFromHandle(const GLuint handle)
{
    return mp_Texture->LoadFromExisting(handle);
}

bool CMaterial::LoadEffect(const gfx::EffectType Type)
{
    m_Effect.Destroy();
    m_Effect.SetType(Type);
    return m_Effect.Init();
}

bool CMaterial::Attach(gfx::CEffect& E, gfxcore::CTexture& T)
{
    mp_Texture = &T;
    m_Effect   = E;
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
    ZEN_ASSERT(mp_Texture != nullptr);
    return mp_Texture->Bind();
}

bool CMaterial::Disable() const
{
    m_Effect.Disable() && mp_Texture->Unbind();
}

bool CMaterial::DisableEffect() const
{
    return m_Effect->Disable();
}

bool CMaterial::DisableTexture() const
{
    ZEN_ASSERT(mp_Texture != nullptr);
    return mp_Texture->Unbind();
}

gfx::CEffect& CMaterial::GetEffect()
{
    return m_Effect;
}

gfxcore::CTexture& CMaterial::GetTexture() const
{
    ZEN_ASSERT(mp_Texture != nullptr);
    return *mp_Texture;
}

void CMaterial::Destroy()
{
    if(!m_egiven) delete mp_Effect;
    if(!m_tgiven) m_Assets.Delete(mp_Texture);
    
    mp_Effect   = nullptr;
    mp_Texture  = nullptr;
}
