zLog& zLog::operator<< (zLog& (zLog::*Fn)())
{
    // Make sure we don't do extra work in release builds.
#ifndef ZEN_DEBUG_BUILD
    if(m_mode == LogMode::ZEN_DEBUG) return (*this);
#endif // ZEN_DEBUG_BUILD

    return (m_enabled ? (this->*Fn)() : *this);
}

template<typename T>
zLog& zLog::operator<<(const T& data)
{
    // Make sure we don't do extra work in release builds.
#ifndef ZEN_DEBUG_BUILD
    if(m_mode == LogMode::ZEN_DEBUG) return (*this);
#endif // ZEN_DEBUG_BUILD

    if(m_enabled) m_str << data;
    return (*this);
}

zLog& zLog::SetMode(const util::LogMode& Mode)
{
    if(m_enabled) m_mode = Mode;
    return (*this);
}

zLog& zLog::SetSystem(const string_t& sys)
{
// Make sure we don't do extra work in release builds.
#ifndef ZEN_DEBUG_BUILD
    if(m_mode == LogMode::ZEN_DEBUG) return (*this);
#endif // ZEN_DEBUG_BUILD

    if(m_enabled) m_system = sys;
    return (*this);
}

void zLog::SetFilename(const string_t& fn)
{
    m_filename = fn;
}

const string_t& zLog::GetSystem() const
{
    return m_system;
}

string_t zLog::GetFilename() const
{
    return m_filename;
}

bool zLog::IsInit() const
{
    return m_init;
}

void zLog::ToggleStdout()
{
    m_stdout = !m_stdout;
}

void zLog::Enable()
{
    m_enabled = true;
}

void zLog::Disable()
{
    m_enabled = false;
}
