uint32_t zAsset::GetFilenameHash() const
{
    return ((m_filename_hash == 0)                          ?
            util::string_hash(m_filename, zAsset::s_seed)   :
            m_filename_hash);
}

uint32_t zAsset::GetAssetID() const
{
    return m_id;
}

const string_t& zAsset::GetFilename() const
{
    return m_filename;
}

const void* const zAsset::GetOwner() const
{
    return mp_owner;
}

const string_t& zAsset::GetError() const
{
    return m_error_str;
}

bool zAsset::IsLoaded() const
{
    return m_loaded;
}

void zAsset::SetOwner(const void* const owner)
{
    mp_owner = owner;
}

void zAsset::SetFilename(const string_t& filename)
{
    ZEN_ASSERT(!filename.empty());

    m_filename = filename;
    m_filename_hash = util::string_hash(filename);
}