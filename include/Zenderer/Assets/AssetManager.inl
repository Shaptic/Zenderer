// Raw asset creation.
template<typename T, typename... S>
T& zAssetManager::Create(const S&... args)
{
    ZEN_ASSERT(this->IsInit());

    m_Log   << m_Log.SetMode(util::LogMode::ZEN_INFO)
            << m_Log.SetSystem("AssetMgr")
            << "Loading raw asset ... ";

    // Create a new original asset.
    T* pAsset = new T(args...);

    // Sanity check, because `new` should throw anyway.
    ZEN_ASSERTM(pAsset != nullptr, "out of memory");

    // Store internally.
    return *this->FinalizeAsset(true, pAsset);
}

// Asset creation from a file, with additional args.
template<typename T, typename... S>
T* zAssetManager::Create(const string_t& filename, const S&... args)
{
    ZEN_ASSERT(this->IsInit());
    ZEN_ASSERT(!filename.empty());

    T* pResult = static_cast<T*>(this->Find(filename, args...));

    // There is no existing asset matching the criteria.
    if(pResult == nullptr)
    {
        m_Log   << m_Log.SetMode(util::LogMode::ZEN_INFO)
                << m_Log.SetSystem("AssetMgr") << "Loading '"
                << filename << "' ... ";

        // Create a new original asset.
        T* pAsset = new T(args...);

        // Make sure we allocated successfully.
        ZEN_ASSERTM(pAsset != nullptr, "out of memory");
        pAsset->mp_Parent = this;

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


template<typename T, typename... S>
T* zAssetManager::Create(const char* const filename, const S&... args)
{
    // The strange casting is a necessity to match the overloaded
    // template specification, rather than the generic S&&... match.
    return this->Create<T>(static_cast<const string_t&>(
                            string_t(filename)
                          ), args...);
}

template<typename T>
T* zAssetManager::Create(const string_t& filename)
{
    return zAssetManager::Create<T>(filename, nullptr);
}

template<typename T>
T* zAssetManager::Create(const char* const filename)
{
    return zAssetManager::Create<T>(static_cast<const string_t&>(
                                        string_t(filename)
                                    ));
}

template<typename T>
T* zAssetManager::Recreate(const T* const Copier, const void* const owner,
                           const void* const settings)
{
    m_Log   << m_Log.SetMode(util::LogMode::ZEN_INFO)
            << m_Log.SetSystem("AssetMgr") << "Copying '"
            << Copier->GetFilename() << "' ... ";

    // Create a new asset.
    T* pAsset = new T(owner, settings);

    // Make sure we allocated successfully.
    ZEN_ASSERTM(pAsset != nullptr, "out of memory");

    // Load the asset from an existing one.
    pAsset->mp_Parent = this;
    return this->FinalizeAsset(pAsset->LoadFromExisting(Copier), pAsset);
}

template<typename T>
T* zAssetManager::Recreate(const string_t& filename, const void* const owner,
                           const void* const settings)
{
    m_Log << m_Log.SetMode(util::LogMode::ZEN_INFO)
          << m_Log.SetSystem("AssetMgr") << "Copying '"
          << filename << "' ... ";

    // Create a new asset.
    T* pAsset = new T(owner, settings);

    // Make sure we allocated successfully.
    ZEN_ASSERTM(pAsset != nullptr, "out of memory");

    // Load the asset from a file on disk.
    pAsset->mp_Parent = this;
    return this->FinalizeAsset(pAsset->LoadFromFile(filename), pAsset);
}

template<typename T>
T* zAssetManager::Recreate(const char* filename, const void* const owner,
                           const void* const settings)
{
    return this->Recreate<T>(string_t(filename), owner, settings);
}

template<typename... S>
zAsset* zAssetManager::Find(const zen::string_t& filename,
                            const void* const owner,
                            const S&... args) const
{
    ZEN_ASSERT(this->IsInit());
    ZEN_ASSERT(!filename.empty());

    m_Log   << m_Log.SetMode(util::LogMode::ZEN_DEBUG)
            << m_Log.SetSystem("AssetMgr") << "Searching for '"
            << filename << "':" << owner << "." << util::zLog::endl;

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

template<typename T>
T* zAssetManager::FinalizeAsset(const bool success, T* pAsset)
{
    if(success)
    {
        m_Log << "Success. ID: " << pAsset->GetAssetID() << "." << util::zLog::endl;

        // Add to containers and return.
        ++pAsset->m_refcount;
        pAsset->mp_Parent = this;
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
