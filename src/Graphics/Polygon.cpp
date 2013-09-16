#include "Zenderer/Graphics/Polygon.hpp"

using namespace zen;
using gfxcore::zRenderer;
using gfxcore::index_t;

using gfx::zPolygon;

zPolygon::zPolygon(asset::zAssetManager& Assets, const size_t preload) :
    m_Assets(Assets), mp_VAO(nullptr), m_Material(Assets),
    mp_MVMatrix(nullptr), m_offset(0), m_internal(false)
{
    m_DrawData.Vertices = nullptr;
    m_DrawData.Indices  = nullptr;
    m_DrawData.icount   = m_DrawData.vcount   = 0;

    m_Material.LoadEffect(gfx::EffectType::NO_EFFECT);
    m_Material.LoadTexture(zRenderer::GetDefaultTexture());

    m_Verts.clear();
    m_Verts.reserve(preload);
}

zPolygon::zPolygon(const zPolygon& Copy) :
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

    m_Verts.clear();
    m_Verts.reserve(Copy.m_Verts.size());
    for(auto i : Copy.m_Verts)
        m_Verts.emplace_back(i);
}

zPolygon::zPolygon(zPolygon&& Move) :
    m_Assets(Move.m_Assets),
    mp_VAO(nullptr), m_Material(m_Assets),
    mp_MVMatrix(Move.mp_MVMatrix), m_offset(0),
    m_internal(false)
{
    m_Material.LoadEffect(gfx::EffectType::NO_EFFECT);
    m_Material.LoadTexture(zRenderer::GetDefaultTexture());

    m_DrawData.vcount   = Move.m_DrawData.vcount;
    m_DrawData.icount   = Move.m_DrawData.icount;
    m_DrawData.Vertices = Move.m_DrawData.Vertices;
    m_DrawData.Indices  = Move.m_DrawData.Indices;

    Move.m_DrawData.Vertices = nullptr;
    Move.m_DrawData.Indices  = nullptr;

    ZEN_ASSERT(m_DrawData.vcount > 0 && m_DrawData.icount > 0);

    m_Verts = std::move(Move.m_Verts);
}

zPolygon::~zPolygon()
{
    if(m_internal) delete mp_VAO;
    m_Verts.clear();
}

void zPolygon::Move(const math::vector_t& Position)
{
    m_Position = Position;
}

void zPolygon::Move(const real_t x, const real_t y, const real_t z /*= 0.0*/)
{
    m_Position = math::vector_t(x, y, z);
}

void zPolygon::AttachMaterial(gfx::zMaterial& Material)
{
    m_Material.LoadEffect(Material.GetEffect().GetType());
    m_Material.LoadTexture(Material.GetTexture());
}

void zPolygon::RemoveMaterial()
{
    m_Material.LoadEffect(gfx::EffectType::NO_EFFECT);
    m_Material.LoadTexture(zRenderer::GetDefaultTexture());
}

void zPolygon::AddVertex(const math::vector_t& Position)
{
    m_Verts.emplace_back(Position);
}

zPolygon& zPolygon::Create()
{
    if(m_Verts.size() <= 2) return (*this);

    uint16_t tris = (m_Verts.size() - 2) * 3;
    gfxcore::index_t* indices = new gfxcore::index_t[tris];

    for(uint16_t i = 0; i < tris; i += 3)
    {
        uint16_t x = i / 3;
        indices[i] = 0;
        indices[i+1] = x + 1;
        indices[i+2] = x + 2;
    }

    m_DrawData.Vertices = new gfxcore::vertex_t[m_Verts.size()];
    m_DrawData.vcount   = m_Verts.size();
    m_DrawData.Indices  = indices;
    m_DrawData.icount   = tris;

    math::vector_t First(m_Verts[0]);
    for(size_t i = 0; i < m_Verts.size(); ++i)
    {
        m_DrawData.Vertices[i].position = std::move(m_Verts[i]);
        m_DrawData.Vertices[i].color    = m_Color;
    }

    m_Verts.clear();
    return (*this);
}

bool zPolygon::Draw(const bool is_bound /*= false*/)
{
    if(mp_VAO == nullptr && !is_bound)
    {
        // Create a vertex array and load our data.
        mp_VAO = new gfxcore::zVertexArray(GL_STATIC_DRAW);
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
        const gfx::zEffect& Effect = m_Material.GetEffect();
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
                      gfxcore::INDEX_TYPE,
                      (void*)(sizeof(index_t) * m_offset)));

    return is_bound ? true : zRenderer::ResetMaterialState() && mp_VAO->Unbind();
}

void zPolygon::LoadIntoVAO(gfxcore::zVertexArray& VAO, const bool keep)
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

void zPolygon::SetColor(const color4f_t& Color)
{
    m_Color = Color;
    for(size_t i = 0; i < m_DrawData.vcount; ++i)
        m_DrawData.Vertices[i].color = Color;
}

void zPolygon::SetIndices(const std::vector<gfxcore::index_t>& Indices)
{
    if(m_DrawData.Indices != nullptr && m_DrawData.icount > 0)
        delete[] m_DrawData.Indices;

    m_DrawData.icount = Indices.size();
    m_DrawData.Indices = new gfxcore::index_t[m_DrawData.icount];
    std::copy(Indices.begin(), Indices.end(), m_DrawData.Indices);
}

uint16_t zPolygon::GetH() const
{
    if(!(m_Verts.size() || m_DrawData.vcount)) return 0;

    // Calculate lowest and highest y-values.
    real_t low, high;
    if(m_DrawData.vcount > 0)
        low = high = m_DrawData.Vertices[0].position.y;
    else
        low = high = m_Verts[0].y;

    for(auto& i : m_Verts)
    {
        low  = math::min<real_t>(low, i.y);
        high = math::max<real_t>(high, i.y);
    }

    for(size_t i = 0; i < m_DrawData.vcount; ++i)
    {
        low  = math::min<real_t>(low,  m_DrawData.Vertices[i].position.y);
        high = math::max<real_t>(high, m_DrawData.Vertices[i].position.y);
    }

    return high - low;
}

uint16_t zPolygon::GetW() const
{
    if(!(m_Verts.size() || m_DrawData.vcount)) return 0;

    // Calculate lowest and highest x-values.
    real_t left, right;
    if(m_DrawData.vcount > 0)
        left = right = m_DrawData.Vertices[0].position.x;
    else
        left = right = m_Verts[0].x;

    for(auto& i : m_Verts)
    {
        left  = math::min<real_t>(left,  i.x);
        right = math::max<real_t>(right, i.x);
    }

    for(size_t i = 0; i < m_DrawData.vcount; ++i)
    {
        left  = math::min<real_t>(left,  m_DrawData.Vertices[i].position.x);
        right = math::max<real_t>(right, m_DrawData.Vertices[i].position.x);
    }

    return right - left;
}

bool zPolygon::IsModifiable() const
{
    return (mp_VAO == nullptr || !mp_VAO->Offloaded());
}

const gfx::zMaterial& zPolygon::GetMaterial() const
{
    return m_Material;
}
