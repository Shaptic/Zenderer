#include "Zenderer/Assets/Asset.hpp"

using namespace zen;

using util::CLog;
using util::LogMode;

using asset::CAsset;

size_t CAsset::s_seed = 0x6915BAD;

CAsset::CAsset(const void* const owner) :
    m_Log(CLog::GetEngineLog()),
    mp_owner(owner), m_filename_hash(0), m_loaded(false),
    m_id(util::hash(this, sizeof(CAsset), CAsset::s_seed))
{
    m_Log   << m_Log.SetMode(LogMode::ZEN_INFO)
            << m_Log.SetSystem("Asset")
            << "Created asset with ID " << m_id << "."
            << CLog::endl;
}

CAsset::~CAsset()
{
    m_Log   << m_Log.SetMode(LogMode::ZEN_INFO)
            << m_Log.SetSystem("Asset")
            << "Asset with ID " << m_id << " destroyed."
            << CLog::endl;
}
