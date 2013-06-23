template<typename T>
T* CAssetManager::Create(const string_t& filename, const void* const owner)
{
    ZEN_ASSERT(this->IsInit());
    ZEN_ASSERT(!filename.empty());
    
    T* pResult = dynamic_cast<T*>(this->Find(filename, owner));
    
    // There is no existing asset matching the criteria.
    if(pResult == nullptr)
    {
        util::g_EngineLog << util::g_EngineLog.SetMode(LogMode::ZEN_INFO)
                          << util::g_EngineLog.SetSystem("AssetManager")
                          << "Loading '" << filename << "' ... ";
        
        // Create a new original asset.
        pAsset = new (g_Alloc.get<T>(1)) T(owner);
        
        // Make sure we allocated successfully.
        ZEN_ASSERTM(pAsset != nullptr, "out of memory");
        
        // Load the asset from disk.
        return this->FinalizeAsset(pAsset->LoadFromFile(Copier), pAsset);
    }
    
    // Return existing asset.
    else
    {
        return pResult;
    }
}

template<typename T>
T* CAssetManager::Recreate(const T* const Copier, const void* const owner)
{
    util::g_EngineLog << util::g_EngineLog.SetMode(LogMode::ZEN_INFO)
                      << util::g_EngineLog.SetSystem("AssetManager")
                      << "Copying '" << filename << "' ... ";
    
    // Create a new asset.
    T* pAsset = new (g_Alloc.get<T>(1)) (owner);
    
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
        util::g_EngineLog << "Success. ID: (" << pAsset->GetID() << ").";
        util::g_EngineLog.Newline();    
        
        // Add to containers and return.
        CAssetManager::sp_allAssets.push_back(pAsset);
        mp_managerAssets.push_back(pAsset);
        return pAsset;
    }
    else
    {
        util::g_EngineLog << util::g_EngineLog.SetMode(LogMode::ZEN_ERROR) << "Failure.";
        util::g_EngineLog.Newline();
        
        // Clean up on failure.
        delete pAsset;
        return (pAsset = nullptr);
    }
}

uint32_t CAssetManager::GetAssetCount() const 
{
    return mp_managerAssets.size();
}
