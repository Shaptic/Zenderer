#include "Zenderer/Utilities/Log.hpp"

using namespace zen;
using util::zLog;

// Dat syntax...
zLog&               // Returns zLog
(
    zLog::*         // Pointer to member function of zLog
    zLog::endl      // Named zLog::endl (thus initializing this static member)
)()                 // No parameters for method
 = &zLog::Newline;  // Assigned to the address of zLog::Newline

zLog::zLog(const string_t& filename, const bool show_stdout) :
    m_mode(LogMode::ZEN_INFO),
    m_system("Log"),
    m_filename(filename),
    m_stdout(show_stdout),
    m_init(false),
    m_enabled(true)
{}

zLog::~zLog()
{
    this->Destroy();
}

bool zLog::Init()
{
    // Don't initialize again w/o a call to Destroy()
    if(this->IsInit()) return false;

    m_log.open(m_filename);
    if(!m_log) return false;

    time_t now;
    time(&now);

    m_log << "[INFO ] Log -- Log initialized on " << ctime(&now);
    this->Newline();

    return (m_init = true);
}

bool zLog::Destroy()
{
    // Can't destroy if we haven't initialized.
    if(!this->IsInit()) return false;

    this->Newline();

    time_t now;
    time(&now);

    m_mode = LogMode::ZEN_INFO;
    m_system = "Log";
    (*this) << "Log closed on " << ctime(&now) << zLog::endl;

    m_log.close();

    return !(m_init = false);
}

zLog& zLog::Newline()
{
    if(!m_enabled || m_str.str().empty() || !this->IsInit())
        return (*this);

#ifndef ZEN_DEBUG_BUILD
    if(m_mode == util::LogMode::ZEN_DEBUG) return (*this);
#endif // ZEN_DEBUG_BUILD

    std::stringstream tmp;

    tmp << "[";

    switch(m_mode)
    {
    case LogMode::ZEN_DEBUG:   tmp << "DEBUG"; break;
    case LogMode::ZEN_INFO:    tmp << "INFO "; break;
    case LogMode::ZEN_ERROR:   tmp << "ERROR"; break;
    case LogMode::ZEN_FATAL:   tmp << "FATAL"; break;
    default:                   tmp << "LOGGR"; break;
    }

    tmp << "] " << m_system << " -- " << m_str.str() << std::endl;

    m_log << tmp.str();
    if(m_stdout) std::cout << tmp.str();

    if(m_mode == LogMode::ZEN_FATAL)
    {
        gfx::error_window(tmp.str().c_str(), "Fatal Error");

        // If these aren't cleared, exit() calls ~zLog() which calls Destroy()
        // which calls Newline() so the message would be output twice.
        m_log.flush();
        m_str.str(std::string());
        exit(1);
    }

    m_log.flush();
    m_str.str(std::string());

    return (*this);
}

zLog& zLog::GetEngineLog()
{
    static zLog Log("Zenderer.log");
    return Log;
}
