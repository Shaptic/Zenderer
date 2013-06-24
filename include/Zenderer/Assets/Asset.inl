uint32_t CAsset::GetFilenameHash() const
{
    return ((m_filename_hash == 0)                          ?
            util::string_hash(m_filename, CAsset::s_seed)   :
            m_filename_hash);
}

uint32_t CAsset::GetID() const
{
    return m_id;
}

const string_t& CAsset::GetFilename() const
{
    return m_filename;
}

const void* const CAsset::GetOwner() const
{
    return mp_owner;
}

const string_t& CAsset::GetError() const
{
    return m_error_str;
}

bool CAsset::IsLoaded() const
{
    return m_loaded;
}

void CAsset::SetOwner(const void* const owner)
{
    mp_owner = owner;
}

void CAsset::SetFilename(const string_t& filename)
{
    ZEN_ASSERT(!filename.empty());

    m_filename = filename;
    m_filename_hash = util::string_hash(filename);
}