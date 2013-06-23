#include "Zenderer/Assets/Asset.hpp"

using namespace zen;

USING_ZENDERER_LOG

using asset::CAsset;

size_t CAsset::s_seed = 0x6915BAD;

CAsset::CAsset(const void* const owner) :
    mp_owner(owner), m_filename_hash(0), m_loaded(false),
    m_id(util::hash(this, sizeof(CAsset), CAsset::s_seed))
{
    g_EngineLog << g_EngineLog.SetMode(LogMode::ZEN_INFO)
                << g_EngineLog.SetSystem("Asset") 
                << "Created asset with ID " << m_id << "."
                << CLog::endl;
}

CAsset::~CAsset()
{
    g_EngineLog << g_EngineLog.SetMode(LogMode::ZEN_INFO)
                << g_EngineLog.SetSystem("Asset") 
                << "Asset with ID " << m_id << " destroyed."
                << CLog::endl;
}
