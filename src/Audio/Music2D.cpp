#include "Zenderer/Audio/Music2D.hpp"

using namespace zen;

using util::zLog;
using util::LogMode;

using sfx::zMusic2D;

zMusic2D::zMusic2D() : m_active(0),
    m_ogg_buffer(nullptr),
    m_format(AL_FORMAT_STEREO16), m_freq(0)
{
    if(m_AL.buffers) delete[] m_AL.buffers;
    m_AL.buffers = new ALuint[zMusic2D::BUFFER_COUNT];
    m_AL.buffer_count = zMusic2D::BUFFER_COUNT;

    ov_clear(&m_ogg);
}

zMusic2D::~zMusic2D()
{
    this->UnloadSource();
}

bool zMusic2D::LoadFromFile(const string_t& filename)
{
    ZEN_ASSERTM(!filename.empty(), "no filename");
    ZEN_ASSERTM(zAudioManager::IsInit(), "OpenAL not initialized");

    m_error_code = AL_NO_ERROR;
    m_error_str  = "No error";

    // Delete existing source
    if(m_AL.source == 0)
    {
        this->UnloadSource();
    }

    // Delete existing stream
    for(size_t i = 0; i < zMusic2D::BUFFER_COUNT; ++i)
    {
        if(m_AL.buffers[i])
        {
            AL(alDeleteBuffers(1, &m_AL.buffers[i]));
            m_AL.buffers[i] = 0;
        }
    }

    // Reset to 0's
    memset(m_AL.buffers, 0, zMusic2D::BUFFER_COUNT * sizeof(ALuint));

    // Open the OGG file.
    FILE* pFile = fopen(filename.c_str(), "rb");

    if(pFile == nullptr)
    {
        m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                << m_Log.SetSystem("Audio") << "Failed to open '"
                << filename << "'." << zLog::endl;
    }

    // Validate proper .ogg format
    if(ov_open_callbacks(pFile, &m_ogg, NULL, 0, OV_CALLBACKS_DEFAULT) < 0)
    {
        ov_clear(&m_ogg);
        fclose(pFile);

        m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                << m_Log.SetSystem("Audio") << "'" << filename
                << "' is not a valid .ogg file." << zLog::endl;

        return (m_loaded = false);
    }

    vorbis_info* pInfo = ov_info(&m_ogg, -1);
    if(pInfo->channels == 1)
        m_format = AL_FORMAT_MONO16;

    m_freq = pInfo->rate;

    AL(alGenBuffers(zMusic2D::BUFFER_COUNT, m_AL.buffers));

    for(size_t i = 0; i < zMusic2D::BUFFER_COUNT; ++i)
        if(!this->FillChunk(m_AL.buffers[i])) return false;

    m_filename = filename;
    return (m_loaded = true);
}

void zMusic2D::Play()
{
    if(!this->IsLoaded())
    {
        m_Log   << m_Log.SetMode(LogMode::ZEN_DEBUG)
                << m_Log.SetSystem("Audio")
                << "No file loaded" << zLog::endl;

        return;
    }

    // Check to see if we are already playing; if so,
    // do nothing. We cannot play from the start since
    // it's a stream.
    if(this->GetAudioState() == AL_PLAYING)
    {
        m_Log   << m_Log.SetMode(LogMode::ZEN_DEBUG)
                << m_Log.SetSystem("Audio")
                << "Already playing '" << m_filename << "'" << zLog::endl;

        return;
    }

    m_AL.source = zAudioManager::CreateSource();
    if(m_AL.source == 0) return;

    m_Log   << m_Log.SetMode(LogMode::ZEN_DEBUG)
            << m_Log.SetSystem("Audio") << "Playing '" << m_filename
            << "' (" << m_AL.source << ")" << zLog::endl;

    AL(alSourcef(m_AL.source, AL_GAIN, m_AL.volume));
    AL(alSourceQueueBuffers(m_AL.source, zMusic2D::BUFFER_COUNT, m_AL.buffers));
    AL(alSourcePlay(m_AL.source));
}

void zMusic2D::Update()
{
    int processed;

    AL(alGetSourcei(m_AL.source, AL_BUFFERS_PROCESSED, &processed));

    while(processed--)
    {
        // Temporary buffer handle
        ALuint buffer;

        // Pop from front of queue
        AL(alSourceUnqueueBuffers(m_AL.source, 1, &buffer));

        // Load from stream
        FillChunk(buffer);

        // Push to back of queue
        AL(alSourceQueueBuffers(m_AL.source, 1, &buffer));
    }
}

bool zMusic2D::Ready() const
{
    int processed = 0;
    AL(alGetSourcei(m_AL.source, AL_BUFFERS_PROCESSED, &processed));
    return (processed > 0);
}

bool zMusic2D::FillChunk(const uint32_t buffer)
{
    uint16_t total = 0;
    int bits = 0;

    if(m_ogg_buffer) delete[] m_ogg_buffer;
    m_ogg_buffer = new char[zMusic2D::READ_SIZE];

    while(total < zMusic2D::READ_SIZE)
    {
        int size = ov_read(&m_ogg, m_ogg_buffer + total,
                           zMusic2D::READ_SIZE - total,
                           /* little endian */ 0,
                           /* 16-bit        */ 2,
                           /* signed        */ 1,
                           &bits);

        if(size > 0) total += size;

        else if(size < 0)
        {
            ov_clear(&m_ogg);
            delete[] m_ogg_buffer;
            m_ogg_buffer = nullptr;

            zAudioManager::OGGError(size);
            return false;
        }

        else break;
    }

    if(total == 0)
    {
        ov_clear(&m_ogg);
        delete[] m_ogg_buffer;
        m_ogg_buffer = nullptr;

        m_Log   << m_Log.SetMode(LogMode::ZEN_DEBUG)
                << m_Log.SetSystem("OpenAL")
                << "Reached end of audio file." << zLog::endl;
        return false;
    }

    AL(alBufferData(buffer, m_format, m_ogg_buffer, total, m_freq));

    return true;
}
