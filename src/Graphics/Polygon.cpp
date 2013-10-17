#include "Zenderer/Graphics/Polygon.hpp"

using namespace zen;
using gfxcore::zRenderer;
using gfxcore::index_t;

using gfx::zPolygon;
using gfx::VAOState;
using gfx::RenderState;

zPolygon::zPolygon(asset::zAssetManager& Assets, const size_t preload) :
    m_Assets(Assets), mp_MVMatrix(nullptr), mp_VAO(nullptr),
    m_offset(0), m_Material(Assets), m_internal(false)
{
    m_DrawData.Vertices = nullptr;
    m_DrawData.Indices  = nullptr;
    m_DrawData.icount   = m_DrawData.vcount = 0;

    m_Material.LoadEffect(gfx::EffectType::NO_EFFECT);
    m_Material.LoadTexture(zRenderer::GetDefaultTexture());

    m_Verts.clear();
    m_Verts.reserve(preload);
}

zPolygon::zPolygon(const zPolygon& Copy) :
    m_Assets(Copy.m_Assets), mp_MVMatrix(nullptr),
    mp_VAO(nullptr), m_offset(0), m_Material(m_Assets),
    m_BoundingBox(Copy.m_BoundingBox), m_internal(false)
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

    m_Verts.reserve(Copy.m_Verts.size());
    for(auto i : Copy.m_Verts) m_Verts.emplace_back(i);

    m_Tris.reserve(Copy.m_Tris.size());
    for(auto i : Copy.m_Tris) m_Tris.emplace_back(i);
}

zPolygon::zPolygon(zPolygon&& Move) :
    m_Assets(Move.m_Assets), mp_MVMatrix(Move.mp_MVMatrix),
    mp_VAO(nullptr), m_offset(0), m_Verts(Move.m_Verts),
    m_Tris(Move.m_Tris), m_Material(m_Assets), m_internal(false)
{
    m_Material.LoadEffect(gfx::EffectType::NO_EFFECT);
    m_Material.LoadTexture(zRenderer::GetDefaultTexture());

    m_DrawData.vcount   = Move.m_DrawData.vcount;
    m_DrawData.icount   = Move.m_DrawData.icount;
    m_DrawData.Vertices = Move.m_DrawData.Vertices;
    m_DrawData.Indices  = Move.m_DrawData.Indices;

    Move.m_DrawData.Vertices = nullptr;
    Move.m_DrawData.Indices  = nullptr;
    Move.mp_MVMatrix         = nullptr;

    ZEN_ASSERT(m_DrawData.vcount > 0 && m_DrawData.icount > 0);
}

zPolygon::~zPolygon()
{
    if(m_DrawData.vcount != 0) delete m_DrawData.Vertices;
    if(m_DrawData.icount != 0) delete m_DrawData.Indices;
    if(m_internal)  delete mp_VAO;
    if(mp_MVMatrix) delete mp_MVMatrix;
    m_Verts.clear();
}

void zPolygon::Move(const math::vector_t& Position)
{
    math::vector_t d = Position - math::vector_t(m_BoundingBox.x,
                                                 m_BoundingBox.y);

    m_BoundingBox.x = Position.x;
    m_BoundingBox.y = Position.y;

    for(auto& i : m_Tris) i = i + d;
}

void zPolygon::Move(const real_t x, const real_t y)
{
    math::vector_t d = math::vector_t(x, y) - math::vector_t(m_BoundingBox.x,
                                                             m_BoundingBox.y);

    m_BoundingBox.x = x;
    m_BoundingBox.y = y;

    for(auto& i : m_Tris) i = i + d;
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

zPolygon& zPolygon::Create(const bool do_triangulation)
{
    if(m_Verts.size() <= 2) return (*this);
    if(m_DrawData.Indices == nullptr || m_DrawData.icount == 0)
    {
        uint16_t tris = (m_Verts.size() - 2) * 3;
        gfxcore::index_t* indices = new gfxcore::index_t[tris];

        for(uint16_t i = 0; i < tris; i += 3)
        {
            uint16_t x = i / 3;
            indices[i] = 0;
            indices[i+1] = x + 1;
            indices[i+2] = x + 2;
        }

        m_DrawData.Indices  = indices;
        m_DrawData.icount   = tris;
    }

    m_DrawData.Vertices = new gfxcore::vertex_t[m_Verts.size()];
    m_DrawData.vcount   = m_Verts.size();

    for(size_t i = 0; i < m_Verts.size(); ++i)
    {
        m_DrawData.Vertices[i].position = std::move(m_Verts[i]);
        m_DrawData.Vertices[i].color    = m_Color;
    }

    m_Tris = do_triangulation ? math::triangulate(m_Verts) : std::move(m_Verts);
    m_Verts.clear();

    // Calculate lowest and highest x/y-values.
    m_BoundingBox.h = this->CalcH();
    m_BoundingBox.w = this->CalcW();

    return (*this);
}

bool zPolygon::Draw(const RenderState& state)
{
    if(mp_VAO == nullptr && state == RenderState::NOT_READY)
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
    if(state == RenderState::NOT_READY)
    {
        if(!mp_VAO->Bind()) return false;

        // Insert our coordinates to transform in the shader.
        // Ignore the Z coordinate because that's only used for depth
        // sorting internally anyway and has no effect on visuals.
        (*mp_MVMatrix)[0][3] = m_BoundingBox.x;
        (*mp_MVMatrix)[1][3] = m_BoundingBox.y;
        // (*mp_MVMatrix)[2][3] = m_Position.z;

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

    return state == RenderState::READY ? true
        : zRenderer::ResetMaterialState() && mp_VAO->Unbind();
}

void zPolygon::LoadIntoVAO(gfxcore::zVertexArray& VAO,
                           const VAOState& state)
{
    if(m_DrawData.Vertices == nullptr ||
       m_DrawData.Indices  == nullptr) return;

    gfxcore::index_t i = VAO.AddData(m_DrawData);
    if(state == VAOState::NO_PRESERVE_DATA)
    {
        delete[] m_DrawData.Vertices;
        delete[] m_DrawData.Indices;
        m_DrawData.Vertices = nullptr;
        m_DrawData.Indices = nullptr;
        m_offset = i;
        mp_VAO = &VAO;
    }
}

bool zPolygon::Collides(const zPolygon& Other, math::cquery_t* q) const
{
    for(size_t i = 0; i < m_Tris.size(); i += 3)
    {
        for(size_t j = 0; j < Other.m_Tris.size(); j += 3)
        {
            math::tri_t t1 = {
                m_Tris[i],
                m_Tris[i+1],
                m_Tris[i+2]
            };

            math::tri_t t2 = {
                Other.m_Tris[j],
                Other.m_Tris[j+1],
                Other.m_Tris[j+2]
            };

            if(math::collides(t1, t2, q))
            {
                if(q != nullptr)
                {
                    q->tri1 = std::move(t1);
                    q->tri2 = std::move(t2);
                    q->collision = true;
                }

                return true;
            }
        }
    }

    return false;
}

bool zPolygon::Collides(const math::aabb_t& other) const
{
    for(size_t i = 0; i < m_Tris.size(); i += 3)
    {
        math::tri_t t = {
            m_Tris[i],
            m_Tris[i+1],
            m_Tris[i+2]
        };

        if(other.collides(t)) return true;
    }

    return false;
}

void zPolygon::SetColor(const color4f_t& Color)
{
    m_Color = Color;
    for(size_t i = 0; i < m_DrawData.vcount; ++i)
        m_DrawData.Vertices[i].color = Color;
}

void zPolygon::SetColor(const real_t r, const real_t g,
                        const real_t b, const real_t a)
{
    this->SetColor(color4f_t(r, g, b, a));
}

void zPolygon::SetIndices(const std::vector<gfxcore::index_t>& Indices)
{
    if(m_DrawData.Indices != nullptr && m_DrawData.icount > 0)
        delete[] m_DrawData.Indices;

    m_DrawData.icount = Indices.size();
    m_DrawData.Indices = new gfxcore::index_t[m_DrawData.icount];
    std::copy(Indices.begin(), Indices.end(), m_DrawData.Indices);
}

int16_t zPolygon::CalcX() const
{
    ZEN_ASSERTM(!m_Verts.empty() || m_DrawData.vcount > 0,
                "polygon has no vertex data");

    int16_t low = m_Verts.empty() ? m_DrawData.Vertices[0].position.x
                                  : m_Verts[0].x;

    for(auto& i : m_Verts)
        low = math::min<int16_t>(low, i.x);

    for(size_t i = 0; i < m_DrawData.vcount; ++i)
        low = math::min<int16_t>(low, m_DrawData.Vertices[i].position.x);

    return low;
}

int16_t zPolygon::CalcY() const
{
    ZEN_ASSERTM(!m_Verts.empty() || m_DrawData.vcount > 0,
                "polygon has no vertex data");

    int16_t low = m_Verts.empty() ? m_DrawData.Vertices[0].position.y
                                  : m_Verts[0].y;

    for(auto& i : m_Verts)
        low = math::min<int16_t>(low, i.y);

    for(size_t i = 0; i < m_DrawData.vcount; ++i)
        low = math::min<int16_t>(low, m_DrawData.Vertices[i].position.y);

    return low;
}

uint16_t zPolygon::CalcH()
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

    return (m_BoundingBox.h = (high - low));
}

uint16_t zPolygon::CalcW()
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

    return (m_BoundingBox.w = (right - left));
}

int zPolygon::GetLowPoint() const
{
    if(m_Verts.empty() && m_DrawData.vcount == 0) return 0;

    int low = m_Verts.empty() ? m_DrawData.Vertices[0].position.y
                              : m_Verts[0].y;

    for(auto& i : m_Verts) low = math::min<int>(low, i.y);
    std::for_each(m_DrawData.Vertices,
                  m_DrawData.Vertices + m_DrawData.vcount,
                  [&low](const gfxcore::vertex_t& v) {
                      low = math::min<int>(low, v.position.y);
                  });

    return low;
}

int zPolygon::GetLeftPoint() const
{
    if(m_Verts.empty() && m_DrawData.vcount == 0) return 0;

    int16_t left = m_Verts.empty() ? m_DrawData.Vertices[0].position.x
                                   : m_Verts[0].x;

    for(auto& i : m_Verts) left = math::min<int>(left, i.x);
    std::for_each(m_DrawData.Vertices,
                  m_DrawData.Vertices + m_DrawData.vcount,
                  [&left](const gfxcore::vertex_t& v) {
                      left = math::min<int>(left, v.position.x);
                  });

    return left;
}

bool zPolygon::IsModifiable() const
{
    return (mp_VAO == nullptr || !mp_VAO->Offloaded());
}

const gfx::zMaterial& zPolygon::GetMaterial() const
{
    return m_Material;
}
