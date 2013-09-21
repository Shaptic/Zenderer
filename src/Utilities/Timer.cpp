#include "Zenderer/Utilities/Timer.hpp"

using std::chrono::duration_cast;

using namespace zen;

using util::zLog;
using util::LogMode;
using util::zTimer;

zTimer::zTimer(const uint16_t frames) :
    m_fps(frames), m_delta(1000.0 / frames) {}

zTimer::~zTimer(){}

util::time_t zTimer::Start()
{
    m_start = clock_t::now();
    return duration_cast<precision_t>(m_start.time_since_epoch()).count();
}

util::time_t zTimer::Finish()
{
    m_end = clock_t::now();
    return duration_cast<precision_t>(m_end.time_since_epoch()).count();
}

util::time_t zTimer::Elapsed()
{
    return duration_cast<precision_t>(m_end - m_start).count();
}

/// Sleeps the current thread (defaults to milliseconds).
void zTimer::Sleep(const util::time_t ticks)
{
    std::this_thread::sleep_for(precision_t(ticks));
}

util::time_t zTimer::Delay()
{
    // Convert the time difference to milliseconds.
    time_t ms = duration_cast<precision_t>(clock_t::now() - m_start).count();

    if(ms < m_delta)
    {
        precision_t pause(time_t(m_delta - ms));
        std::this_thread::sleep_for(pause);

#if defined(ZEN_SHOW_DELAY) && defined(ZEN_DEBUG_BUILD)
        zLog& L = zLog::GetEngineLog();
        L << L.SetMode(LogMode::ZEN_DEBUG) << L.SetSystem("Timer")
          << "Delaying for " << pause.count() << "ms." << zLog::endl;
#endif // ZEN_SHOW_DELAY
    }

    return ms;
}

util::time_t zTimer::GetTime() const
{
    return duration_cast<precision_t>(
        clock_t::now().time_since_epoch()).count();
}

void zTimer::SetFrameRate(const uint16_t fps)
{
    m_fps = fps;

    // This relies on precision_t to be a millisecond.
    m_delta = 1000.0 / m_fps;
}
