template<typename T>
T* zAssetManager::Create(const string_t& filename, const void* const owner)
{
    ZEN_ASSERT(this->IsInit());
    ZEN_ASSERT(!filename.empty());

    T* pResult = static_cast<T*>(this->Find(filename, owner));

    // There is no existing asset matching the criteria.
    if(pResult == nullptr)
    {
        m_Log   << m_Log.SetMode(util::LogMode::ZEN_INFO)
                << m_Log.SetSystem("AssetMgr") << "Loading '"
                << filename << "' ... ";

        // Create a new original asset.
        T* pAsset = new T(owner);

        // Make sure we allocated successfully.
        ZEN_ASSERTM(pAsset != nullptr, "out of memory");

        // Load the asset from disk.
        return this->FinalizeAsset(pAsset->LoadFromFile(filename), pAsset);
    }

    // Return existing asset.
    else
    {
        ++pResult->m_refcount;
        m_Log   << m_Log.SetMode(util::LogMode::ZEN_INFO)
                << m_Log.SetSystem("AssetMgr") << "Using existing asset for '"
                << filename << "' ... " << util::zLog::endl;
        return pResult;
    }
}

template<typename T>
T* zAssetManager::Create(const char* const filename, const void* const owner)
{
    return this->Create<T>(string_t(filename), owner);
}

template<typename T>
T* zAssetManager::Create(const void* const owner)
{
    ZEN_ASSERT(this->IsInit());

    m_Log   << m_Log.SetMode(util::LogMode::ZEN_INFO)
            << m_Log.SetSystem("AssetMgr")
            << "Loading raw asset ... ";

    // Create a new original asset.
    T* pAsset = new T(owner);

    // Sanity check, because `new` should throw anyway.
    ZEN_ASSERTM(pAsset != nullptr, "out of memory");

    // Store internally.
    return this->FinalizeAsset(true, pAsset);
}

template<typename T>
T* zAssetManager::Recreate(const T* const Copier, const void* const owner)
{
    m_Log   << m_Log.SetMode(util::LogMode::ZEN_INFO)
            << m_Log.SetSystem("AssetMgr") << "Copying '"
            << filename << "' ... ";

    // Create a new asset.
    T* pAsset = new T(owner);

    // Make sure we allocated successfully.
    ZEN_ASSERTM(pAsset != nullptr, "out of memory");

    // Load the asset from an existing one.
    return this->FinalizeAsset(pAsset->LoadFromExisting(Copier), pAsset);
}

template<typename T>
T* zAssetManager::FinalizeAsset(const bool success, T* pAsset)
{
    if(success)
    {
        m_Log << "Success. ID: " << pAsset->GetAssetID() << "." << util::zLog::endl;

        // Add to containers and return.
        ++pAsset->m_refcount;
        zAssetManager::sp_allAssets.push_back(pAsset);
        mp_managerAssets.push_back(pAsset);
        return pAsset;
    }
    else
    {
        m_Log   << m_Log.SetMode(util::LogMode::ZEN_ERROR)
                << "Failure." << util::zLog::endl;

        // Clean up on failure.
        delete pAsset;
        return (pAsset = nullptr);
    }
}

uint32_t zAssetManager::GetAssetCount() const
{
    return mp_managerAssets.size();
}

std::list<zAsset*>::const_iterator zAssetManager::cbegin()
{
    return mp_managerAssets.begin();
}

std::list<zAsset*>::const_iterator zAssetManager::cend()
{
    return mp_managerAssets.end();
}
