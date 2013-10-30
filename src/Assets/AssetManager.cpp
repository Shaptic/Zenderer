#include "Zenderer/Assets/AssetManager.hpp"

using namespace zen;
using namespace asset;

using util::zLog;
using util::LogMode;

std::list<zAsset*> zAssetManager::sp_allAssets;

zAssetManager::zAssetManager() :
    zSubsystem("AssetMgr"),
    m_Log(zLog::GetEngineLog())
{
    mp_managerAssets.clear();
}

zAssetManager::~zAssetManager()
{
    this->Destroy();
}

bool zAssetManager::Init()
{
    if(m_init) return false;
    if(zAsset::s_seed == 0) zAsset::s_seed = rand() % RAND_MAX;
    return (m_init = true);
}

bool zAssetManager::Destroy()
{
    if(this->IsInit())
    {
        for(auto& i : mp_managerAssets)
            delete i;

        mp_managerAssets.clear();
    }

    return !(m_init = false);
}

bool zAssetManager::Delete(zAsset* const pAsset)
{
    if(!this->IsInit() || pAsset == nullptr) return false;

    m_Log.SetSystem("AssetMgr");

    // Verify that this is our asset to delete.
    auto b = mp_managerAssets.begin(), e = mp_managerAssets.end();
    for( ; b != e; )
    {
        zAsset& Asset = **b;
        if(&Asset == pAsset && Asset.GetOwner() == pAsset->GetOwner())
        {
            ZEN_ASSERTM(Asset.m_loaded, "deleting unloaded asset");

            if(Asset.m_refcount > 1)
            {
                --Asset.m_refcount;
                m_Log   << m_Log.SetMode(util::LogMode::ZEN_DEBUG)
                        << "Decreasing reference count for asset ("
                        << Asset.GetFilename() << "): "
                        << Asset.m_refcount << '.' << zLog::endl;
                return false;
            }

            m_Log   << m_Log.SetMode(LogMode::ZEN_INFO)
                    << "Deleting asset: " << Asset.GetAssetID()
                    << " (" << Asset.GetFilename() << ")." << zLog::endl;

            auto s = zAssetManager::sp_allAssets.begin(),
                 t = zAssetManager::sp_allAssets.end();

            // Remove from global storage
            for( ; s != t; )
            {
                if(*s == pAsset && (*s)->GetOwner() == pAsset->GetOwner())
                {
                    // It will be deleted outside of the loop,
                    // so just remove from list.
                    sp_allAssets.erase(s);
                    break;
                }

                ++s;
            }

            delete &Asset;
            mp_managerAssets.erase(b);
            return true;
        }

        ++b;
    }

    // We didn't find the asset to delete.
    return false;
}

bool zAssetManager::Delete(const uint32_t index)
{
    ZEN_ASSERT(this->IsInit());
    ZEN_ASSERTM(index < this->GetAssetCount() && index >= 0,
                "index out of range");

    std::list<zAsset*>::iterator it = mp_managerAssets.begin();
    std::advance(it, index);

    if(it != mp_managerAssets.end())
    {
        m_Log   << m_Log.SetSystem("AssetMgr")
                << m_Log.SetMode(LogMode::ZEN_INFO)
                << "Deleting asset: " << (*it)->GetAssetID()
                << '(' << (*it)->GetFilename() << ")." << zLog::endl;

        mp_managerAssets.erase(it);
        return true;
    }

    return false;
}

zAsset* zAssetManager::Find(const zen::string_t& filename,
                            const void* const owner) const
{
    ZEN_ASSERT(this->IsInit());
    ZEN_ASSERT(!filename.empty());

    m_Log   << m_Log.SetMode(LogMode::ZEN_DEBUG)
            << m_Log.SetSystem("AssetMgr") << "Searching for '"
            << filename << "':" << owner << "." << zLog::endl;

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

zAsset* zAssetManager::Find(const assetid_t id) const
{
    m_Log   << m_Log.SetMode(LogMode::ZEN_DEBUG)
            << m_Log.SetSystem("AssetMgr")
            << "Searching for asset with ID " << id << '.' << zLog::endl;

    for(auto b : mp_managerAssets)
    {
        if(b->GetAssetID() == id) return b;
    }

    return nullptr;
}

uint32_t zAssetManager::GetGlobalAssetCount()
{
    return zAssetManager::sp_allAssets.size();
}
