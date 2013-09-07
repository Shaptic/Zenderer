#include "Zenderer/CoreGraphics/Drawable.hpp"

using namespace zen;
using gfxcore::zDrawable;

zDrawable::zDrawable(asset::zAssetManager& Assets) :
    m_Assets(Assets), mp_VAO(nullptr), m_Material(Assets),
    mp_MVMatrix(nullptr), m_offset(0), m_internal(false)
{
    m_DrawData.Vertices = nullptr;
    m_DrawData.Indices  = nullptr;
    m_DrawData.icount   =
    m_DrawData.vcount   = 0;

    m_Material.LoadEffect(gfx::EffectType::NO_EFFECT);
    m_Material.LoadTexture(zRenderer::GetDefaultTexture());
}

zDrawable::zDrawable(const zDrawable& Copy) :
    m_Assets(Copy.m_Assets),
    mp_VAO(nullptr), m_Material(m_Assets),
    mp_MVMatrix(nullptr), m_offset(0),
    m_internal(false)
{
    m_Material.LoadEffect(gfx::EffectType::NO_EFFECT);
    m_Material.LoadTexture(zRenderer::GetDefaultTexture());

    m_DrawData.vcount   = Copy.m_DrawData.vcount;
    m_DrawData.icount   = Copy.m_DrawData.icount;
    m_DrawData.Vertices = new gfxcore::vertex_t[m_DrawData.vcount];
    m_DrawData.Indices  = new gfxcore::index_t[m_DrawData.icount];

    ZEN_ASSERT(m_DrawData.vcount > 0 && m_DrawData.icount > 0);

    for(size_t i = 0; i < m_DrawData.icount; ++i)
        m_DrawData.Indices[i] = Copy.m_DrawData.Indices[i];

    for(size_t v = 0; v < m_DrawData.vcount; ++v)
        m_DrawData.Vertices[v] = Copy.m_DrawData.Vertices[v];

    if(Copy.mp_MVMatrix != nullptr)
        mp_MVMatrix = new math::matrix4x4_t(*Copy.mp_MVMatrix);
}

zDrawable::zDrawable(zDrawable&& Copy) :
    m_Assets(Copy.m_Assets),
    mp_VAO(nullptr), m_Material(m_Assets),
    mp_MVMatrix(Copy.mp_MVMatrix), m_offset(0),
    m_internal(false)
{
    m_Material.LoadEffect(gfx::EffectType::NO_EFFECT);
    m_Material.LoadTexture(zRenderer::GetDefaultTexture());

    m_DrawData.vcount   = Copy.m_DrawData.vcount;
    m_DrawData.icount   = Copy.m_DrawData.icount;
    m_DrawData.Vertices = Copy.m_DrawData.Vertices;
    m_DrawData.Indices  = Copy.m_DrawData.Indices;

    Copy.m_DrawData.Vertices = nullptr;
    Copy.m_DrawData.Indices  = nullptr;

    ZEN_ASSERT(m_DrawData.vcount > 0 && m_DrawData.icount > 0);
}

zDrawable::~zDrawable()
{
    if(m_internal) delete mp_VAO;
}

void zDrawable::Move(const math::vector_t& Position)
{
    m_Position = Position;
}

void zDrawable::Move(const real_t x, const real_t y, const real_t z /*= 0.0*/)
{
    m_Position = math::vector_t(x, y, z);
}

void zDrawable::AttachMaterial(gfx::zMaterial& Material)
{
    m_Material.LoadEffect(Material.GetEffect().GetType());
    m_Material.LoadTexture(Material.GetTexture());
}

void zDrawable::RemoveMaterial()
{
    m_Material.LoadEffect(gfx::EffectType::NO_EFFECT);
    m_Material.LoadTexture(zRenderer::GetDefaultTexture());
}

void zDrawable::SetColor(const color4f_t& Color)
{
    for(size_t i = 0; i < m_DrawData.vcount; ++i)
    {
        m_DrawData.Vertices[i].color = Color;
    }
}

bool zDrawable::Draw(const bool is_bound /*= false*/)
{
    if(mp_VAO == nullptr && !is_bound)
    {
        // Create a vertex array and load our data.
        mp_VAO = new zVertexArray(GL_STATIC_DRAW);
        mp_VAO->Init();
        m_offset = mp_VAO->AddData(m_DrawData);
        if(!mp_VAO->Offload()) return false;

        // Create our model-view matrix.
        mp_MVMatrix = new math::matrix4x4_t(math::
                                matrix4x4_t::GetIdentityMatrix());

        // So we can differentiate between a VAO from a `zScene`
        // and the one we made ourselves.
        m_internal = true;
    }

    // If something isn't previously bound, we bind the VAO
    // and the material. If no material, use global default.
    if(!is_bound)
    {
        if(!mp_VAO->Bind()) return false;

        // Insert our coordinates to transform in the shader.
        // Ignore the Z coordinate because that's only used for depth
        // sorting internally anyway and has no effect on visuals.
        (*mp_MVMatrix)[0][3] = m_Position.x;
        (*mp_MVMatrix)[1][3] = m_Position.y;
      //(*mp_MVMatrix)[2][3] = m_Position.z;

        // Bind our material. If we haven't set one, the default will be used.
        // We need a default texture because otherwise the color would
        // always be black due to the way the shader works.
        gfx::zEffect& Effect = m_Material.GetEffect();
        m_Material.Enable();

        // All effects have these parameters in the vertex shader.
        if(!Effect.SetParameter("mv", *mp_MVMatrix) ||
           !Effect.SetParameter("proj", zRenderer::GetProjectionMatrix()))
        {
            zRenderer::ResetMaterialState();
            return false;
        }
    }

    GL(glDrawElements(GL_TRIANGLES, m_DrawData.icount,
                      INDEX_TYPE, (void*)(sizeof(index_t) * m_offset)));

    return !is_bound ?
        (zRenderer::ResetMaterialState() && mp_VAO->Unbind()) :
        true;
}

void zDrawable::LoadIntoVAO(gfxcore::zVertexArray& VAO, const bool keep)
{
    if(m_DrawData.Vertices == nullptr ||
       m_DrawData.Indices  == nullptr) return;

    gfxcore::index_t i = VAO.AddData(m_DrawData);
    if(!keep)
    {
        delete[] m_DrawData.Vertices;
        delete[] m_DrawData.Indices;
        m_DrawData.Vertices = nullptr;
        m_DrawData.Indices = nullptr;
        m_offset = i;
        mp_VAO = &VAO;
    }
}

bool zDrawable::IsModifiable() const
{
    return (mp_VAO == nullptr || !mp_VAO->Offloaded());
}
