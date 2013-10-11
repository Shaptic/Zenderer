#include "Zenderer/Graphics/Scene.hpp"

using namespace zen;
using namespace gfx;

using util::zLog;
using util::LogMode;
using gfxcore::zRenderer;
using gfxcore::BlendFunc;

zScene::zScene(const uint16_t w, const uint16_t h, asset::zAssetManager& Mgr) :
    zSubsystem("Scene"), m_Assets(Mgr),
    m_Log(util::zLog::GetEngineLog()), m_FBO1(w, h), m_FBO2(w, h),
    m_Camera(0.0, 0.0, 0.0), m_lighting(false),
    m_ppfx(false), m_through(false)
{
}

zScene::~zScene()
{
    this->Destroy();
}

bool zScene::Init()
{
    return  m_Assets.IsInit()   && m_Geometry.Init() &&
            m_FBO1.Init()       && m_FBO2.Init();
}

bool zScene::Destroy()
{
    return this->Clear()    && m_FBO1.Destroy() &&
           m_FBO2.Destroy() && m_Geometry.Destroy();
}

obj::zEntity& zScene::AddEntity()
{
    obj::zEntity* pNew = new obj::zEntity(m_Assets);
    m_allEntities.push_back(pNew);
    return *m_allEntities.back();
}

zLight& zScene::AddLight(const LightType& Type)
{
    zLight* pNew = new zLight(m_Assets, Type, m_FBO1.GetHeight());
    pNew->Init();
    m_allLights.push_back(pNew);
    return *m_allLights.back();
}

zEffect& zScene::AddEffect(const EffectType& Type)
{
    zEffect* pNew = new zEffect(Type, m_Assets);
    if(pNew->Init())
    {
        pNew->Enable();
        pNew->SetParameter("mv", math::matrix4x4_t::GetIdentityMatrix());
        pNew->SetParameter("proj", zRenderer::GetProjectionMatrix());
        pNew->Disable();
    }

    m_allPPFX.push_back(pNew);
    return *m_allPPFX.back();
}

obj::zEntity& zScene::InsertEntity(const uint32_t index)
{
    obj::zEntity* pNew = new obj::zEntity(m_Assets);
    auto i = m_allEntities.begin();
    for(size_t j = 0, s = m_allEntities.size();
        j < s && j != index; ++j, ++i);
        // No -op

    m_allEntities.insert(i, pNew);
    return *pNew;
}

bool zScene::RemoveEntity(const obj::zEntity& Obj)
{
    auto i = m_allEntities.begin(),
         j = m_allEntities.end();

    for( ; i != j; ++i)
    {
        if(*i == &Obj)
        {
            delete *i;
            m_allEntities.erase(i);
            return true;
        }
    }

    return false;
}

bool zScene::RemoveEntity(const uint32_t index)
{
    if(!this->IsValidEntityIndex(index)) return false;

    auto i = m_allEntities.begin();
    for(size_t j = 0; j <= index; ++j, ++i);
        // No-op

    delete *i;
    m_allEntities.erase(i);
    return true;
}

bool zScene::ShiftEntity(obj::zEntity& Obj, const uint32_t index)
{
    if(!this->IsValidEntityIndex(index)) return false;
    auto i = m_allEntities.begin(),
         j = m_allEntities.end();

    for( ; i != j; ++i)
    {
        if(*i == &Obj)
        {
            m_allEntities.erase(i);
            break;
        }
    }
    if(i == j) return false;

    auto tmp = m_allEntities.begin();
    std::advance(tmp, index);
    m_allEntities.insert(tmp, &Obj);
    return true;
}

bool zScene::Clear()
{
    for(auto i : m_allEntities) delete i;
    for(auto i : m_allLights) delete i;
    for(auto i : m_allPPFX) delete i;

    m_allEntities.clear();
    m_allLights.clear();
    m_allPPFX.clear();

    return m_Geometry.Clear();
}

bool zScene::Render()
{
    // Called every frame because there is no more appropriate
    // time to call it. Things won't be offloaded multiple times.
    for(auto& i : m_allEntities)
    {
        i->Offload(m_Geometry, false);
    }

    m_Geometry.Offload();

    // Clear our frame buffers from the last drawing.
    // We will be rendering to FBO1 at first.
    color4f_t Clear = color4f_t(0.0, 0.0, 0.0);
    if(m_through) Clear.a = 0.0;
    m_FBO2.Bind(); m_FBO2.Clear(Clear);
    m_FBO1.Bind(); m_FBO1.Clear(Clear);

    // Set the standard blending state.
    bool blend = zRenderer::BlendOperation(BlendFunc::IS_ENABLED);
    zRenderer::BlendOperation(BlendFunc::STANDARD_BLEND);

    // All geometry is stored here.
    m_Geometry.Bind();

    // Prepare for primitive rendering.
    const zEffect& E = zRenderer::GetDefaultEffect();

    // Commence individual primitive rendering.
    for(auto& i : m_allEntities)
    {
        i->Update();
        if(!i->m_enabled) continue;

        // Adjust for the camera.
        i->Move(i->GetPosition() + m_Camera);

        // Set the matrix for transformation.
        const math::matrix4x4_t& Tmp = i->GetTransformation();

        for(const auto& j : *i)
        {
            auto& M = j->GetMaterial();
            M.Enable();
            M.GetEffect().SetParameter("mv", Tmp);
            j->Draw(true);
        }

        // Move back to original position.
        i->Move(i->GetPosition() - m_Camera);
    }

    // Shortcut reference.
    gfxcore::zVertexArray& FS = zRenderer::GetFullscreenVBO();

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
        zRenderer::EnableTexture(final_texture);
        zRenderer::BlendOperation(BlendFunc::ADDITIVE_BLEND);

        for(auto& i : m_allLights)
        {
            i->Enable();

            // Move with scene
            i->Adjust(m_Camera.x, m_Camera.y);

            FS.Draw();

            // Restore state
            i->Adjust(-m_Camera.x, -m_Camera.y);
            i->Disable();
        }

        final_texture = m_FBO2.GetTexture();
        zRenderer::BlendOperation(BlendFunc::STANDARD_BLEND);
    }

    // Ping-pong post-processing effects.
    // This means one is drawn to FBO1, then FBO1's result
    // is used to draw on FBO2, etc.
    if(m_ppfx)
    {
        // If there was lighting, the first texture is
        // the second FBO.
        zRenderTarget& One = m_lighting ? m_FBO2 : m_FBO1;
        zRenderTarget& Two = m_lighting ? m_FBO1 : m_FBO2;

        auto i = m_allPPFX.begin(), j = m_allPPFX.end();

        for(size_t c = 0; i != j; ++i, ++c)
        {
            auto& FX = *i;
            bool even = ((c & 0x1) == 0);
            if(even)    Two.Bind();
            else        One.Bind();

            FX->Enable();
            zRenderer::EnableTexture(final_texture);
            FS.Draw();
            FX->Disable();

            final_texture = even ? Two.GetTexture() : One.GetTexture();
        }
    }

    // Now we have the final scene data in `final_texture`

    // Doesn't matter which we disable.
    m_FBO1.Unbind();

    E.Enable();
    zRenderer::EnableTexture(final_texture);
    if(!m_through) zRenderer::BlendOperation(BlendFunc::DISABLE_BLEND);
    GL(glDisable(GL_DEPTH_TEST));

    // Make sure the right data is set.
    E.SetParameter("mv", math::matrix4x4_t::GetIdentityMatrix());
    E.SetParameter("proj", zRenderer::GetProjectionMatrix());

    FS.Draw();

    zRenderer::EnableTexture(0);
    zRenderer::ResetMaterialState();

    if(!blend && m_through) zRenderer::BlendOperation(BlendFunc::DISABLE_BLEND);

    return true;
}

int32_t zScene::GetEntityIndex(const obj::zEntity& D)
{
    int32_t index = -1;
    for(auto i : m_allEntities)
    {
        ++index;
        if(i == &D) return index;
    }

    return index;
}

bool zScene::IsValidEntityIndex(int32_t i)
{
    return (i >= 0 && i < m_allEntities.size());
}
