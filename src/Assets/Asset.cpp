#include "Zenderer/Assets/Asset.hpp"

using namespace zen;
using util::zLog;
using util::LogMode;

using asset::zAsset;

size_t zAsset::s_seed = 0x6915BAD;  // lol

zAsset::zAsset(const void* const owner,
               const void* const settings) :
    m_Log(zLog::GetEngineLog()), m_filename_hash(0),
    m_loaded(false), mp_Parent(nullptr), m_refcount(0),
    m_id(util::hash(this, sizeof(zAsset), zAsset::s_seed)),
    mp_owner(owner) {}

zAsset::~zAsset() {}

bool zAsset::LoadFromExisting(const zAsset* const pCopy)
{
    m_error_str     = pCopy->m_error_str;
    m_filename      = pCopy->m_filename;
    m_filename_hash = pCopy->m_filename_hash;
    mp_owner        = pCopy->mp_owner;
    m_loaded        = pCopy->m_loaded;

    return true;
}
