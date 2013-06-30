#include "Zenderer/Utilities/Timer.hpp"

using std::chrono::duration_cast;

using namespace zen;

using util::CLog;
using util::LogMode;
using util::CTimer;

CTimer::CTimer(const uint16_t frames) :
    m_fps(frames), m_delta(1000.0 / frames) {}

CTimer::~CTimer(){}

util::time_t CTimer::Start()
{
    m_start = clock_t::now();
    return duration_cast<precision_t>(m_start.time_since_epoch()).count();
}

util::time_t CTimer::Finish()
{
    m_end = clock_t::now();
    return duration_cast<precision_t>(m_end.time_since_epoch()).count();
}

util::time_t CTimer::Elapsed()
{
    return duration_cast<precision_t>(m_end - m_start).count();
}

/// Sleeps the current thread (defaults to milliseconds). 
void CTimer::Sleep(const util::time_t ticks)
{
    std::this_thread::sleep_for(precision_t(ticks));
}

util::time_t CTimer::Delay()
{
    // Convert the time difference to milliseconds.
    time_t ms = duration_cast<precision_t>(clock_t::now() - m_start).count();

    if(ms < m_delta)
    {
        precision_t pause(time_t(m_delta - ms));
        std::this_thread::sleep_for(pause);

#if defined(ZEN_SHOW_DELAY) && defined(_DEBUG)
        CLog& L = CLog::GetEngineLog();
        L << L.SetMode(LogMode::ZEN_DEBUG) << L.SetSystem("Timer")
          << "Delaying for " << pause.count() << "ms." << CLog::endl;
#endif // ZEN_SHOW_DELAY
    }

    return ms;
}

util::time_t CTimer::GetTime() const
{
    return duration_cast<precision_t>(
        clock_t::now().time_since_epoch()).count();
}

void CTimer::SetFrameRate(const uint16_t fps)
{
    m_fps = fps;

    // This relies on precision_t to be a millisecond.
    m_delta = 1000.0 / m_fps;
}
