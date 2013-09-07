#include "Zenderer/Audio/AudioManager.hpp"

using namespace zen;

using util::zLog;
using util::LogMode;

using sfx::zAudioManager;

uint32_t    zAudioManager::s_available[256];
bool        zAudioManager::s_init = false;

bool zAudioManager::Init()
{
    if(!zAudioManager::IsInit())
    {
        memset(s_available, 0, zAudioManager::AVAILABLE_BUFFERS);

        alutInit(NULL, NULL);
        if(alutGetError() != ALUT_ERROR_NO_ERROR)
            return false;

        // Clear error codes.
        alGetError();

        s_init = true;
    }

    return s_init;
}

int zAudioManager::GetAvailableSourceIndex()
{
    static uint16_t last = 0;

    // Check if it's already free
    if(s_available[last] == 0)
        return last;

    // Check if the next one is free (likely)
    if(last < AVAILABLE_BUFFERS && !s_available[last + 1])
    {
        return ++last;
    }

    // Iterate from the start to find a free one
    for(size_t i = 0; i < AVAILABLE_BUFFERS; ++i)
    {
        if(!s_available[i])
        {
            last = i;
            return last;
        }
    }

    // Nothing...
    return -1;
}

int zAudioManager::GetAvailableSource(const uint16_t index)
{
    ZEN_ASSERT(index >= 0 && index < AVAILABLE_BUFFERS);

    return s_available[index];
}

ALuint zAudioManager::CreateSource()
{
    zLog& g_EngineLog = zLog::GetEngineLog();

    int index = zAudioManager::GetAvailableSourceIndex();

    if(index == -1)
    {
        g_EngineLog << g_EngineLog.SetMode(LogMode::ZEN_ERROR)
                    << g_EngineLog.SetSystem("Audio")
                    << "OpenAL source limit reached." << zLog::endl;
        return 0;
    }

    AL(alGenSources(1, &s_available[index]));
    return s_available[index];
}

bool zAudioManager::FreeSource(ALuint src)
{
    if(!src) return true;

    for(size_t i = 0; i < AVAILABLE_BUFFERS; ++i)
    {
        if(s_available[i] == src)
        {
            AL(alDeleteSources(1, &src));
            s_available[i] = 0;
            return true;
        }
    }

    return false;
}

bool zAudioManager::alCheck(const char*     expr,
                            const uint32_t  line,
                            const char*     file)
{
    ALenum error_code = alGetError();

    if(error_code == AL_NO_ERROR) return true;

    // So it's not recreated on the stack every time.
    static std::stringstream ss;

    // Clear existing data (since static).
    ss.str(std::string());

    // Format: CODE (STRING).
    ss << error_code << " (" << alGetString(error_code) << ").";

    // Express our discontent.
    util::runtime_assert(false, expr, line, file, ss.str().c_str());

    return false;
}

void zAudioManager::OGGError(const int error_code)
{
    zLog& g_EngineLog = zLog::GetEngineLog();

    g_EngineLog << g_EngineLog.SetMode(LogMode::ZEN_ERROR)
                << g_EngineLog.SetSystem("Audio")
                << "OGG Error #" << error_code << " (";

    switch(error_code)
    {
    case OV_EREAD:
        g_EngineLog << "error reading from file";
        break;

    case OV_ENOTVORBIS:
        g_EngineLog << "invalid OGG data";
        break;

    case OV_EVERSION:
        g_EngineLog << "Vorbis version mismatch";
        break;

    case OV_EBADHEADER:
        g_EngineLog << "invalid Vorbis header";
        break;

    case OV_EFAULT:
        g_EngineLog << "memory corruption";
        break;

    default:
        g_EngineLog << "unknown";
        break;
    }

    g_EngineLog << ")." << zLog::endl;
}

bool zAudioManager::IsInit()
{
    return s_init;
}