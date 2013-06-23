#include "Zenderer/Audio/Music2D.hpp"

using namespace zen;

USING_ZENDERER_LOG

using sfx::CMusic2D;

CMusic2D::CMusic2D() : m_active(0),
    m_ogg_buffer(nullptr),
    m_format(AL_FORMAT_STEREO16), m_freq(0)
{
    g_Alloc.Free(m_AL.buffers);
    m_AL.buffers = g_Alloc.get<ALuint>(CMusic2D::BUFFER_COUNT);
    m_AL.buffer_count = CMusic2D::BUFFER_COUNT;

    ov_clear(&m_ogg);
}

CMusic2D::~CMusic2D()
{
    this->UnloadSource();
}

bool CMusic2D::LoadFromFile(const string_t& filename)
{   
    ZEN_ASSERTM(!filename.empty(), "no filename");
    ZEN_ASSERTM(CAudioManager::IsInit(), "OpenAL not initialized");
    
    m_error_code = AL_NO_ERROR;
    m_error_str  = "No error";
    
    // Delete existing source
    if(m_AL.source == 0)
    {
        this->UnloadSource();
    }
    
    // Delete existing stream
    for(size_t i = 0; i < CMusic2D::BUFFER_COUNT; ++i)
    {
        if(m_AL.buffers[i])
        {
            AL(alDeleteBuffers(1, &m_AL.buffers[i]));
            m_AL.buffers[i] = 0;
        }
    }
    
    // Reset to 0's
    memset(m_AL.buffers, 0, CMusic2D::BUFFER_COUNT * sizeof(ALuint));

    // Open the OGG file.
    FILE* pFile = fopen(filename.c_str(), "rb");
    
    if(pFile == nullptr)
    {
        g_EngineLog << g_EngineLog.SetMode(LogMode::ZEN_ERROR)
                    << g_EngineLog.SetSystem("Audio")
                    << "Failed to open '" << filename << "'."
                    << CLog::endl;
    }
    
    // Validate proper .ogg format
    if(ov_open_callbacks(pFile, &m_ogg, NULL, 0, OV_CALLBACKS_DEFAULT) < 0)
    {
        ov_clear(&m_ogg);
        fclose(pFile);
        
        g_EngineLog << g_EngineLog.SetMode(LogMode::ZEN_ERROR)
                    << g_EngineLog.SetSystem("Audio")
                    << "'" << filename << "' is not a valid .ogg file."
                    << CLog::endl;
        
        return (m_loaded = false);
    }
    
    vorbis_info* pInfo = ov_info(&m_ogg, -1);
    if(pInfo->channels == 1)
        m_format = AL_FORMAT_MONO16;
        
    m_freq = pInfo->rate;
    
    AL(alGenBuffers(CMusic2D::BUFFER_COUNT, m_AL.buffers));
    
    for(size_t i = 0; i < CMusic2D::BUFFER_COUNT; ++i)
        if(!this->FillChunk(m_AL.buffers[i])) return false;

    m_filename = filename;
    return (m_loaded = true);
}

void CMusic2D::Play()
{
    if(!this->IsLoaded())
    {
        g_EngineLog << g_EngineLog.SetMode(LogMode::ZEN_DEBUG)
                    << g_EngineLog.SetSystem("Audio")
                    << "No file loaded" << CLog::endl;

        return;
    }
    
    // Check to see if we are already playing; if so, 
    // do nothing. We cannot play from the start since
    // it's a stream.
    if(this->GetAudioState() == AL_PLAYING)
    {
        g_EngineLog << g_EngineLog.SetMode(LogMode::ZEN_DEBUG)
                    << g_EngineLog.SetSystem("Audio")
                    << "Already playing '" << m_filename << "'"
                    << CLog::endl;
        
        return;
    }
    
    m_AL.source = CAudioManager::CreateSource();
    if(m_AL.source == -1) return;

    g_EngineLog << g_EngineLog.SetMode(LogMode::ZEN_DEBUG)
                << g_EngineLog.SetSystem("Audio")
                << "Playing '" << m_filename << "' ("
                << m_AL.source << ")" << CLog::endl;

    AL(alSourcef(m_AL.source, AL_GAIN, m_AL.volume));
    AL(alSourceQueueBuffers(m_AL.source, CMusic2D::BUFFER_COUNT, m_AL.buffers));
    AL(alSourcePlay(m_AL.source));
}

void CMusic2D::Update()
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

bool CMusic2D::Ready() const
{
    int processed = 0;
    AL(alGetSourcei(m_AL.source, AL_BUFFERS_PROCESSED, &processed));
    return (processed > 0);
}

bool CMusic2D::FillChunk(const uint32_t buffer)
{
    uint16_t total = 0;
    int bits = 0;
    
    if(m_ogg_buffer) g_Alloc.Free(m_ogg_buffer);
    m_ogg_buffer = g_Alloc.get<char>(CMusic2D::READ_SIZE);
    
    while(total < CMusic2D::READ_SIZE)
    {
        int size = ov_read(&m_ogg, m_ogg_buffer + total, 
                           CMusic2D::READ_SIZE - total,
                           /* little endian */ 0,
                           /* 16-bit        */ 2,
                           /* signed        */ 1,
                           &bits);

        if(size > 0) total += size;
        
        else if(size < 0) 
        {
            ov_clear(&m_ogg);
            g_Alloc.Free(m_ogg_buffer);
            m_ogg_buffer = nullptr;
            
            CAudioManager::OGGError(size);
            return false;
        }
        
        else break;
    }
    
    if(total == 0)
    {
        ov_clear(&m_ogg);
        g_Alloc.Free(m_ogg_buffer);
        m_ogg_buffer = nullptr;
        
        g_EngineLog << g_EngineLog.SetMode(LogMode::ZEN_DEBUG)
                    << g_EngineLog.SetSystem("OpenAL")
                    << "Reached end of audio file." << CLog::endl;
        return false;
    }
    
    AL(alBufferData(buffer, m_format, m_ogg_buffer, total, m_freq));

    return true;
}
