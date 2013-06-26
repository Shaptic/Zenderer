// Thoughts:
// 
// If a primitive is not created by a scene (and hence has a VBO specified)
// it can create it's own internal mini-scene for rendering.

enum class RenderState : uint16_t
{
    ZEN_NORMAL_RENDER,
    ZEN_OFFSCREEN_RENDER,
    ZEN_ALPHA_RENDER,
    ZEN_LIGHTING_RENDER,
    ZEN_POSTPROCESS_RENDER
};

class ZEN_API CSceneManager
{
public:
    CSceneManager(const CSettings& Settings);
    virtual ~CSceneManager();
    
    /// Only the scenes can modify graphical API state.
    friend class gfx::CScene;
    
protected:
    virtual void SaveRenderState();
    virtual void PrepareRenderState(const RenderState& Type)
    {
        switch(Type)
        {
        case RenderState::ZEN_NORMAL_RENDER:
            // @todo: set identity matrix, proj matrix from window.
            m_DefaultShader.Enable();
            m_FullscreenQuad.Prepare();
            m_FinalTarget.GetTexture().Bind();
            
            m_FullscreenQuad.Draw();
            
            // Return to previous state.
            m_FinalTarget.GetTexture().Unbind();
            m_FullscreenQuad.ResetState();
            m_DefaultShader.Disable();
            break;
            
        case RenderState::OFFSCREEN_RENDER:
            
        }
        
        m_LastState = Type;
    }
    
    // Uses last render state.
    virtual void ResetRenderState();
    
    // CScene::Render() :
    // ...
    // gfxcore::CTexture* pWrapper =
    //      m_Assets.Create<gfxcore::CTexture*>(this);
    // pWrapper->LoadFromTexture(final_texture);
    // m_Renderer.SetFinalTexture(pWrapper);
    // ...
    
    void SetFinalTexture(const asset::CTexture* pTexture)
    {
        ZEN_ASSERT(pTexture != nullptr);
        
        m_FBOTexture = *pTexture;
    }
    
    void UpdateConfiguration();
    
    virtual inline bool Enable(const int flag);
    virtual inline bool Disable(const int flag);

    virtual void PrepareAlphaRenderState();
    virtual void PreparePostFXRenderState();
    virtual void PrepareFBORenderState();
    virtual void PrepareLightingRenderState();
    
    obj::CQuad          m_FullscreenQuad;
    gfx::CEffect        m_DefaultShader;
    gfx::CRenderTarget  m_FinalScene;
    
    RenderState         m_LastState;
}