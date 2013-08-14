#include "Zenderer/Assets/AssetManager.hpp"

using namespace zen;
using namespace asset;

using util::CLog;
using util::LogMode;

std::list<CAsset*> CAssetManager::sp_allAssets;

CAssetManager::CAssetManager() :
    m_Log(CLog::GetEngineLog()),
    CSubsystem("AssetMgr")
{
    mp_managerAssets.clear();
}

CAssetManager::~CAssetManager()
{
    this->Destroy();
}

bool CAssetManager::Init()
{
    if(m_init) return false;
    if(CAsset::s_seed == 0) CAsset::s_seed = rand() % RAND_MAX;
    return (m_init = true);
}

bool CAssetManager::Destroy()
{
    if(this->IsInit())
    {
        while(!mp_managerAssets.empty())
            this->Delete(*mp_managerAssets.begin());

        mp_managerAssets.clear();
    }

    return !(m_init = false);
}

bool CAssetManager::Delete(CAsset* const pAsset)
{
    ZEN_ASSERT(this->IsInit());
    if(pAsset == nullptr) return false;

    m_Log.SetSystem("AssetMgr");

    // Verify that this is our asset to delete.
    auto b = mp_managerAssets.begin(), e = mp_managerAssets.end();
    for( ; b != e; )
    {
        if(*b == pAsset && (*b)->GetOwner() == pAsset->GetOwner())
        {
            if((*b)->m_refcount > 1)
            {
                --(*b)->m_refcount;
                m_Log   << m_Log.SetMode(util::LogMode::ZEN_DEBUG)
                        << "Decreasing reference count for asset ("
                        << (*b)->GetFilename() << "): "
                        << (*b)->m_refcount << '.' << util::CLog::endl;
                return false;
            }

            m_Log   << m_Log.SetMode(LogMode::ZEN_INFO)
                    << "Deleting asset: " << (*b)->GetAssetID()
                    << " (" << (*b)->GetFilename() << ")." << CLog::endl;

            delete *b;
            mp_managerAssets.erase(b);

            auto s = CAssetManager::sp_allAssets.begin(),
                 t = CAssetManager::sp_allAssets.end();

            // Remove from global storage
            for( ; s != t; )
            {
                if(*s == pAsset && (*s)->GetOwner() == pAsset->GetOwner())
                {
                    // It has already been deleted so just remove from list.
                    sp_allAssets.erase(s);
                    break;
                }

                ++s;
            }

            return true;
        }

        ++b;
    }

    // We didn't find the asset to delete.
    return false;
}

bool CAssetManager::Delete(const uint32_t index)
{
    ZEN_ASSERT(this->IsInit());
    ZEN_ASSERTM(index < this->GetAssetCount() && index >= 0,
                "index out of range");

    if(this->GetAssetCount() == 0) return false;

    std::list<CAsset*>::iterator it = mp_managerAssets.begin();
    for(size_t i = 0; i < index; ++i, ++it);

    if(it != mp_managerAssets.end())
    {
        m_Log   << m_Log.SetSystem("AssetMgr")
                << m_Log.SetMode(LogMode::ZEN_INFO)
                << "Deleting asset: " << (*it)->GetAssetID()
                << '(' << (*it)->GetFilename() << ")." << CLog::endl;

        mp_managerAssets.erase(it);
    }

    return true;
}

CAsset* CAssetManager::Find(const zen::string_t& filename,
                            const void* const owner) const
{
    ZEN_ASSERT(this->IsInit());
    ZEN_ASSERT(!filename.empty());

    m_Log   << m_Log.SetMode(LogMode::ZEN_DEBUG)
            << m_Log.SetSystem("AssetMgr") << "Searching for '"
            << filename << "':" << owner << "." << CLog::endl;

    uint32_t hash = util::string_hash(filename);

    // Speed comparisons: http://ideone.com/eaIbCB
    // Above is out-dated: http://ideone.com/droMcn
    for(auto b : mp_managerAssets)
    {
        if(b->GetFilenameHash() == hash && b->GetOwner() == owner)
        {
            return b;
        }
    }

    return nullptr;
}

CAsset* CAssetManager::Find(const assetid_t id) const
{
    m_Log   << m_Log.SetMode(LogMode::ZEN_DEBUG)
            << m_Log.SetSystem("AssetMgr")
            << "Searching for asset with ID " << id << '.' << CLog::endl;

    for(auto b : mp_managerAssets)
    {
        if(b->GetAssetID() == id) return b;
    }

    return nullptr;
}

uint32_t CAssetManager::GetGlobalAssetCount()
{
    return CAssetManager::sp_allAssets.size();
}
