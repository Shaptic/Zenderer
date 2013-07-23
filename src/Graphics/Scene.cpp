#include "Zenderer/Graphics/Scene.hpp"

using namespace zen;
using namespace gfx;

using util::CLog;
using util::LogMode;
using gfxcore::CRenderer;

CScene::CScene(const uint16_t w, const uint16_t h, asset::CAssetManager& Mgr) :
    CSubsystem("Scene"), m_Assets(Mgr),
    m_Log(util::CLog::GetEngineLog()), m_FBO1(w, h), m_FBO2(w, h),
    m_lighting(false), m_ppfx(false)
{
}

CScene::~CScene()
{
    this->Destroy();
}

bool CScene::Init()
{
    return  m_Assets.IsInit()   && m_Geometry.Init() &&
            m_FBO1.Init()       && m_FBO2.Init();
}

bool CScene::Destroy()
{
    bool ret = m_FBO1.Destroy() && m_FBO2.Destroy() && m_Geometry.Destroy();

    for(auto& i : m_allEntities)delete i;
    for(auto& i : m_allPPFX)    delete i;
    for(auto& i : m_allLights)  delete i;

    m_allEntities.clear();
    m_allPPFX.clear();
    m_allLights.clear();

    return ret;
}

obj::CEntity& CScene::AddEntity()
{
    obj::CEntity* pNew = new obj::CEntity(m_Assets);
    m_allEntities.push_back(pNew);
    return *m_allEntities.back();
}

/// @todo Fix hard-coded 800.
CLight& CScene::AddLight(const LightType& Type)
{
    CLight* pNew = new CLight(m_Assets, Type, 800);
    pNew->Init();
    m_allLights.push_back(pNew);
    return *m_allLights.back();
}

CEffect& CScene::AddEffect(const EffectType& Type)
{
    CEffect* pNew = new CEffect(Type, m_Assets);
    pNew->Init();
    m_allPPFX.push_back(pNew);
    return *m_allPPFX.back();
}

obj::CEntity& CScene::InsertEntity(const uint32_t index)
{
    obj::CEntity* pNew = new obj::CEntity(m_Assets);
    auto i = m_allEntities.begin();
    size_t j = 0, s = m_allEntities.size();
    for(size_t j = 0, s = m_allEntities.size();
        j < s && j != index; ++j, ++i);
        m_allEntities.insert(i, pNew);
    return *pNew;
}

bool CScene::Render()
{
    // We keep only one matrix instance and just
    // modify it for every object.
    math::matrix4x4_t MV(math::matrix4x4_t::GetIdentityMatrix());

    // Called every frame because there is no more appropriate
    // time to call it. Things won't be offloaded multiple times.
    auto i = m_allEntities.begin(), j = m_allEntities.end();
    for( ; i != j; ++i)
    {
        (*i)->Offload(m_Geometry, false);
    }

    m_Geometry.Offload();

    // Clear our frame buffers from the last drawing.
    // We will be rendering to FBO1 at first.
    m_FBO2.Bind(); m_FBO2.Clear();
    m_FBO1.Bind(); m_FBO1.Clear();

    // Set the standard blending state.
    bool blend = CRenderer::BlendOperation(BlendFunc::IS_ENABLED);
    CRenderer::BlendOperation(BlendFunc::STANDARD_BLEND);

    // All geometry is stored here.
    m_Geometry.Bind();

    // Prepare for primitive rendering.
    const CMaterial& M = CRenderer::GetDefaultMaterial();
    CEffect& E = const_cast<CMaterial&>(M).GetEffect();
    M.Enable();

    // Commence individual primitive rendering.
    i = m_allEntities.begin();
    for( ; i != j; ++i)
    {
        MV.Translate((*i)->GetPosition() + m_Camera);
        E.SetParameter("mv", MV);

        auto a = (*i)->cbegin(), b = (*i)->cend();
        for( ; a != b; ++a)
        {
            (*a)->GetMaterial().Enable();
            (*a)->Draw(true);
        }
    }

    M.Disable();

    // Shortcut reference.
    gfxcore::CVertexArray& FS = CRenderer::GetFullscreenVBO();

    // Primitive rendering is complete.
    // Now, render lights with additive blending.
    GLuint final_texture = m_FBO1.GetTexture();

    if(m_lighting)
    {
        // We want to render to the entire scene.

        // Lighting renders ON the geometry, so we
        // bind the FBO1 texture to render onto. The
        // final result ends up on the FBO2 texture.
        m_FBO2.Bind();
        CRenderer::EnableTexture(final_texture);
        CRenderer::BlendOperation(BlendFunc::ADDITIVE_BLEND);

        auto i = m_allLights.begin(),
            j = m_allLights.end();

        for( ; i != j; ++i)
        {
            CLight& L = *(*i);
            L.Enable();

            // Move with scene
            L.Adjust(m_Camera.x, m_Camera.y);

            FS.Draw();

            // Restore state
            L.Adjust(-m_Camera.x, -m_Camera.y);
            L.Disable();
        }

        final_texture = m_FBO2.GetTexture();
        CRenderer::BlendOperation(BlendFunc::STANDARD_BLEND);
    }

    // Ping-pong post-processing effects.
    // This means one is drawn to FBO1, then FBO1's result
    // is used to draw on FBO2, etc.
    if(m_ppfx)
    {
        // If there was lighting, the first texture is
        // the second FBO.
        CRenderTarget& One = m_lighting ? m_FBO2 : m_FBO1;
        CRenderTarget& Two = m_lighting ? m_FBO1 : m_FBO2;

        auto i = m_allPPFX.begin(),
            j = m_allPPFX.end();

        for(size_t c = 0; i != j; ++i, ++c)
        {
            bool even = ((c & 0x1) == 0);
            if(even)    Two.Bind();
            else        One.Bind();

            (*i)->Enable();
            CRenderer::EnableTexture(final_texture);
            FS.Draw();
            (*i)->Disable();

            final_texture = even ? Two.GetTexture() : One.GetTexture();
        }
    }

    // Now we have the final scene data in `final_texture`

    // Doesn't matter which we disable.
    m_FBO1.Unbind();

    E.Enable();
    CRenderer::EnableTexture(final_texture);
    CRenderer::BlendOperation(BlendFunc::DISABLE_BLEND);
    GL(glDisable(GL_DEPTH_TEST));

    // Make sure the right data is set.
    E.SetParameter("mv", math::matrix4x4_t::GetIdentityMatrix());
    E.SetParameter("proj", CRenderer::GetProjectionMatrix());

    FS.Draw();

    CRenderer::EnableTexture(0);
    CRenderer::ResetMaterialState();

    return true;
}

int32_t CScene::GetEntityIndex(const obj::CEntity& D)
{
    auto i = m_allEntities.begin(),
        j = m_allEntities.end();

    int32_t index = -1;
    for( ; i != j; ++i, ++index)
    {
        if((*i) == &D) return index;
    }

    return index;
}

bool CScene::IsValidEntityIndex(int32_t i)
{
    return (i > 0 && i < m_allEntities.size());
}
