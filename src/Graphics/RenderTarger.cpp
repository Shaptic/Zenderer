#include "Zenderer/Graphics/RenderTarget.hpp"

using namespace zen;
using gfx::CRenderTarget;

CRenderTarget::CRenderTarget(const math::rect_t& Dimensions) :
    CGLSubsystem("RenderTarget"), m_Log(CLog::GetEngineLog()),
    m_Viewport(Dimensions.w, Dimensions.h) 
{
}

CRenderTarget::CRenderTarget(const uint16_t w, const uint16_t h) : 
    CGLSubsystem("RenderTarget"), m_Viewport(w, h)
{
}

CRenderTarget::~CRenderTarget()
{
    this->Destroy();
    if(m_rbos != nullptr) delete[] m_rbos;
}

bool CRenderTarget::Init()
{
    if(m_init) this->Destroy();
    
    // Store current render target's viewport.
    GLint view[4];
    GL(glGetIntegerv(GL_VIEWPORT, view));
    m_OldViewport.x = view[2];
    m_OldViewport.y = view[3];
    
    // Create frame buffer.
    GL(glGenFramebuffers(1, &m_fbo));
    GL(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
    
    // Create render target texture.
    GL(glGenTextures(1, &m_texture));
    GL(glBindTexture(GL_TEXTURE_2D, m_texture));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_Viewport.x, 
            m_Viewport.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));
    
    // Attach texture to frame buffer.
    GL(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, m_texture, 0));
            
    // Check status.
    GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        m_error_str = "The frame buffer set up did not complete.";
        m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                << m_Log.SetSystem("RenderTarget") << m_error_str 
                << " Error code: " << status << CLog::endl;
    }
    
    // Unbind our things.
    GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GL(glBindTexture(GL_TEXTURE_2D, 0));
    
    return m_init = (status == GL_FRAMEBUFFER_COMPLETE);
}

bool CRenderTarget::Destroy()
{
    m_init = false;
    this->Unbind();
    GL(glDeleteFramebuffers(1, &m_fbo));
    GL(glDeleteTextures(1, &m_texture));
    if(m_rbo_count > 0) GL(glDeleteRenderbuffers(m_rbo_count, m_rbos));
    m_rbo_count = 0;
    delete[] m_rbos;
    m_rbos = nullptr;
}

bool CRenderTarget::Bind() const
{
    // Bind the framebuffer and set our viewport.
    GL(glBindFramebuffer(GL_FRAMEBUFFER, m_fbo));
    GL(glViewport(0, 0, m_Viewport.x, m_Viewport.y));
    return (m_bound = true);
}

bool CRenderTarget::Unbind() const
{
    m_bound = false;
    // Unbind the framebuffer and reset the viewport.
    GL(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    GL(glViewport(0, 0, m_OldViewport.x, m_OldViewport.y));
    return true;
}

bool CRenderTarget::Clear()
{
    if(m_bound)
    {
        GL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        GL(glClearColor(0.f, 0.f, 0.f, 1.f));
    }
    
    return m_bound;
}

bool CRenderTarget::AttachDepthBuffer()
{
    // Create the depth buffer.
    if(!this->Bind()) return false;
    
    GLuint rb = 0;
    GL(glGenRenderbuffers(1, &rb));
    GL(glBindRenderbuffer(GL_RENDERBUFFER, rb));
    GL(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,  
            width, height));
    GL(glFramebufferRenderbuffer(GL_FRAMEBUFFER,  GL_DEPTH_ATTACHMENT, 
            GL_RENDERBUFFER, rb));

    // Check status.
    GLuint status = glCheckFramebufferStatus(GL_FRAMEBUFFER);            
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        m_error_str = "The depth buffer could not be attached.";
        m_Log   << m_Log.SetMode(LogMode::ZEN_ERROR)
                << m_Log.SetSystem("RenderTarget") << m_error_str 
                << " Error code: " << status << CLog::endl;
    }
    else
    {
        // Expand array if necessary 
        if(m_rbos != nullptr)
        {
            GLuint* tmp = new GLuint[m_rbo_count + 1];
            memcpy(tmp, m_rbos, m_rbo_count * sizeof(GLuint));
            delete[] m_rbos;
            m_rbos = tmp;
        }
        
        // Add to internal render buffer array.
        m_rbos[m_rbo_count++] = rb;
    }
    
    GL(glBindRenderbuffer(GL_RENDERBUFFER, 0));
    return (this->Unbind() && status == GL_FRAMEBUFFER_COMPLETE);
}

GLuint CRenderTarget::GetObjectHandle() const
{
    return m_fbo;
}

GLuint CRenderTarget::GetTexture() const
{
    return m_texture;
}
