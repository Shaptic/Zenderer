CLog& CLog::operator<< (CLog& (CLog::*Fn)())
{
    // Make sure we don't do extra work in release builds.
#ifndef ZEN_DEBUG_BUILD
    if(m_mode == LogMode::ZEN_DEBUG) return (*this);
#endif // ZEN_DEBUG_BUILD

    return (this->*Fn)();
}

template<typename T>
CLog& CLog::operator<<(const T& data)
{
    // Make sure we don't do extra work in release builds.
#ifndef ZEN_DEBUG_BUILD
    if(m_mode == LogMode::ZEN_DEBUG) return (*this);
#endif // ZEN_DEBUG_BUILD

    m_str << data;
    return (*this);
}

CLog& CLog::SetMode(const util::LogMode& Mode)
{
    m_mode = Mode;
    return (*this);
}

CLog& CLog::SetSystem(const string_t& sys)
{
// Make sure we don't do extra work in release builds.
#ifndef ZEN_DEBUG_BUILD
    if(m_mode == LogMode::ZEN_DEBUG) return (*this);
#endif // ZEN_DEBUG_BUILD

    m_system = sys;
    return (*this);
}

void CLog::SetFilename(const string_t& fn)
{
    m_filename = fn;
}

const string_t& CLog::GetSystem() const
{
    return m_system;
}

string_t CLog::GetFilename() const
{
    return m_filename;
}

bool CLog::IsInit() const
{
    return m_init;
}
