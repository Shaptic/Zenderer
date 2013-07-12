#include "Zenderer/Graphics/Material.hpp"

using namespace zen;

using util::CLog;
using util::LogMode;
using gfx::CMaterial;

CMaterial::CMaterial(asset::CAssetManager* Assets) :
    m_Assets(Assets == nullptr ? CEffect::s_DefaultManager : *Assets),
    m_Log(CLog::GetEngineLog()), mp_Texture(nullptr),
    mp_Effect(nullptr), m_egiven(false), m_tgiven(false)
{
}

CMaterial::CMaterial(gfxcore::CTexture& Texture,
                      gfx::CEffect& Effect,
                      asset::CAssetManager* Assets) :
    m_Assets(Assets == nullptr ? CEffect::s_DefaultManager : *Assets),
    m_Log(CLog::GetEngineLog()), mp_Texture(&Texture),
    mp_Effect(&Effect), m_egiven(false), m_tgiven(false)
{
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
    
    mp_Effect = new gfx::CEffect(gfx::EffectType::CUSTOM_EFFECT);
    
    if(!mp_Effect->LoadCustomEffect(vs, fs) || 
      (mp_Texture = m_Assets.Create<gfxcore::CTexture>(tx)) == nullptr)
    {
        delete mp_Effect;
        m_Assets.Delete(mp_Texture);
        return false;
    }
    
    return (m_tgiven = m_egiven = true);
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
        valid = this->LoadEffect(EffectType::CUSTOM_EFFECT) &&
            mp_Effect->LoadCustomEffect(Parser.GetValue("vshader"),
                                        Parser.GetValue("fshader"));
    }

    f.seekg(start);
    return valid && f;
}

bool CMaterial::LoadTextureFromFile(const string_t& filename)
{
    mp_Texture = m_Assets.Create<gfxcore::CTexture>(filename);
    if(mp_Texture != nullptr) m_tgiven = false;
    
    return !m_tgiven;
}

bool CMaterial::LoadEffect(const gfx::EffectType Type)
{
    mp_Effect = new CEffect(Type);
    if(Type != EffectType::CUSTOM_EFFECT)
    {
        if(!mp_Effect->Init())
        {
            delete mp_Effect;
            mp_Effect = nullptr;
            return false;
        }
        
        m_egiven = false;
    }
    
    return true;
}

bool CMaterial::Attach(gfx::CEffect& E, gfxcore::CTexture& T)
{
    mp_Texture = &T;
    mp_Effect  = &E;
    return (m_egiven = m_tgiven = true);
}

bool CMaterial::Enable() const
{
    bool ret = true;
    if(mp_Effect)   ret = mp_Effect->Enable();
    if(mp_Texture)  ret = ret && mp_Texture->Bind();

    return ret;
}

bool CMaterial::EnableEffect() const
{
    return (mp_Effect != nullptr && mp_Effect->Enable());
}

bool CMaterial::EnableTexture() const
{
    return (mp_Texture != nullptr && mp_Texture->Bind());
}

bool CMaterial::Disable() const
{
    bool ret = true;
    if(mp_Effect)   ret = mp_Effect->Disable();
    if(mp_Texture)  ret = ret && mp_Texture->Unbind();
    return ret;
}

bool CMaterial::DisableEffect() const
{
    return (mp_Effect != nullptr && mp_Effect->Disable());
}

bool CMaterial::DisableTexture() const
{
    return (mp_Texture != nullptr && mp_Texture->Unbind());
}

gfx::CEffect* CMaterial::GetEffect()
{
    return mp_Effect;
}

gfxcore::CTexture* CMaterial::GetTexture() const
{
    return mp_Texture;
}

void CMaterial::Destroy()
{
    if(!m_egiven) delete mp_Effect;
    if(!m_tgiven) m_Assets.Delete(mp_Texture);
    
    mp_Effect   = nullptr;
    mp_Texture  = nullptr;
}
