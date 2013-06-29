template<typename T>
T* CAssetManager::Create(const string_t& filename, const void* const owner)
{
    ZEN_ASSERT(this->IsInit());
    ZEN_ASSERT(!filename.empty());

    T* pResult = static_cast<T*>(this->Find(filename, owner));

    // There is no existing asset matching the criteria.
    if(pResult == nullptr)
    {
        m_Log   << m_Log.SetMode(LogMode::ZEN_INFO)
                << m_Log.SetSystem("AssetManager") << "Loading '"
                << filename << "' ... " << CLog::endl;

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
        return pResult;
    }
}

template<typename T>
T* CAssetManager::Create(const void* const owner)
{
    ZEN_ASSERT(this->IsInit());

    m_Log   << m_Log.SetMode(LogMode::ZEN_INFO)
            << m_Log.SetSystem("AssetManager")
            << "Loading raw asset ... " << CLog::endl;

    // Create a new original asset.
    T* pAsset = new T(owner);

    // Sanity check, because `new` should throw anyway.
    ZEN_ASSERTM(pAsset != nullptr, "out of memory");

    // Store internally.
    return this->FinalizeAsset(true, pAsset);
}

template<typename T>
T* CAssetManager::Recreate(const T* const Copier, const void* const owner)
{
    m_Log   << m_Log.SetMode(LogMode::ZEN_INFO)
            << m_Log.SetSystem("AssetManager") << "Copying '"
            << filename << "' ... " << CLog::endl;

    // Create a new asset.
    T* pAsset = new T(owner);

    // Make sure we allocated successfully.
    ZEN_ASSERTM(pAsset != nullptr, "out of memory");

    // Load the asset from an existing one.
    return this->FinalizeAsset(pAsset->LoadFromExisting(Copier), pAsset);
}

template<typename T>
T* CAssetManager::FinalizeAsset(const bool success, T* pAsset)
{
    if(success)
    {
        m_Log << "Success. ID: (" << pAsset->GetID() << ")." << CLog::endl;

        // Add to containers and return.
        CAssetManager::sp_allAssets.push_back(pAsset);
        mp_managerAssets.push_back(pAsset);
        return pAsset;
    }
    else
    {
        m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                << "Failure." << CLog::endl;

        // Clean up on failure.
        delete pAsset;
        return (pAsset = nullptr);
    }
}

uint32_t CAssetManager::GetAssetCount() const
{
    return mp_managerAssets.size();
}
