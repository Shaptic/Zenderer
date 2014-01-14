#include "Zenderer/Graphics/Quad.hpp"

using namespace zen;
using gfx::zQuad;

zQuad::zQuad(asset::zAssetManager& Mgr, const math::rect_t& Size) :
    zPolygon(Mgr), m_inv(false), m_rep(false)
{
    m_BoundingBox = Size;
}

zQuad::zQuad(asset::zAssetManager& Mgr, const uint16_t w, const uint16_t h) :
    zPolygon(Mgr), m_inv(false), m_rep(false)
{
    m_BoundingBox = math::rect_t(0, 0, w, h);
}

zQuad::zQuad(asset::zAssetManager& Assets, const GLuint handle,
             const gfx::EffectType Effect) :
    zPolygon(Assets), m_inv(false), m_rep(false)
{
    gfx::zMaterial Material(Assets);
    ZEN_ASSERT(Material.LoadEffect(Effect) &&
               Material.LoadTextureFromHandle(handle));
    this->Resize(Material.GetTexture().GetWidth(),
                 Material.GetTexture().GetHeight());
    this->AttachMaterial(Material);
}

zQuad::zQuad(asset::zAssetManager& Assets, gfxcore::zTexture& Texture,
             const gfx::EffectType Effect) :
    zPolygon(Assets), m_inv(false), m_rep(false)
{
    gfx::zMaterial Material(Assets);
    ZEN_ASSERT(Material.LoadEffect(Effect) &&
               Material.LoadTexture(Texture));
    this->Resize(Texture.GetWidth(), Texture.GetHeight());
    this->AttachMaterial(Material);
}

zQuad::zQuad(const zQuad& Copy) : zPolygon(Copy)
{
    m_inv = Copy.m_inv;
    m_rep = Copy.m_rep;
}

zQuad::~zQuad() {}

gfx::zPolygon& zQuad::Create(const bool triangulate)
{
    if(m_DrawData.Vertices == nullptr)
    {
        m_DrawData.Vertices = new gfxcore::vertex_t[4];
        m_DrawData.vcount = 4;
    }

    if(!m_inv)
    {
        this->LoadRegularVertices();
        this->LoadRegularTC();
    }
    else
    {
        this->LoadInvertedVertices();
        this->LoadInvertedTC();
    }

    for(size_t i = 0; i < m_DrawData.vcount; ++i)
        m_DrawData.Vertices[i].color = m_Color;

    if(m_DrawData.Indices == nullptr)
    {
        m_DrawData.Indices = new gfxcore::index_t[6];
        m_DrawData.icount = 6;
    }

    m_DrawData.Indices[0] = 0;
    m_DrawData.Indices[1] = 1;
    m_DrawData.Indices[2] = 3;
    m_DrawData.Indices[3] = 3;
    m_DrawData.Indices[4] = 1;
    m_DrawData.Indices[5] = 2;

    m_Tris.reserve(m_DrawData.icount);
    for(uint8_t i = 0; i < m_DrawData.icount; ++i)
        m_Tris.push_back(m_DrawData.Vertices[m_DrawData.Indices[i]].position);

    m_Verts.clear();
    return (*this);
}

void zQuad::Resize(const math::vectoru16_t& Size)
{
    this->Resize(Size.x, Size.y);
}

void zQuad::Resize(const uint16_t w, const uint16_t h)
{
    // Offload()'ed already.
    if(!this->IsModifiable()) return;

    m_BoundingBox.w = w;
    m_BoundingBox.h = h;
}

bool zQuad::Collides(const zPolygon& Other, math::cquery_t* q) const
{
    const math::aabb_t us(m_BoundingBox);
    for(size_t i = 0; i < Other.GetTriangulation().size(); i += 3)
    {
        math::tri_t t = {
            Other.GetTriangulation()[i],
            Other.GetTriangulation()[i + 1],
            Other.GetTriangulation()[i + 2]
        };

        if(us.collides(t))
        {
            if(q != nullptr)
            {
                q->box1 = us;
                q->tri2 = t;
                q->collision = true;
            }

            return true;
        }
    }

    return false;
}

bool zQuad::Collides(const zQuad& Other, math::cquery_t* q) const
{
    math::aabb_t us(m_BoundingBox), them(Other.m_BoundingBox);
    if(us.collides(them))
    {
        if(q != nullptr)
        {
            q->box1 = us;
            q->box2 = them;
            q->collision = true;
        }

        return true;
    }

    return false;
}

bool zQuad::Collides(const math::aabb_t& other)
{
    return other.collides(m_BoundingBox);
}

void zQuad::SetInverted(const bool flag)
{
    m_inv = flag;
}

void zQuad::SetRepeating(const bool flag)
{
    m_rep = flag;
}

void zQuad::FlipOn(const gfx::Axis& axes)
{
    m_flips = axes;
}

void zQuad::LoadRegularVertices()
{
    if(m_flips & gfx::Axis::Y)
    {
        m_DrawData.Vertices[0].position = math::vector_t(0, m_BoundingBox.h);
        m_DrawData.Vertices[1].position = math::vector_t(m_BoundingBox.w,
                                                         m_BoundingBox.h);
        m_DrawData.Vertices[2].position = math::vector_t(m_BoundingBox.w, 0);
        m_DrawData.Vertices[3].position = math::vector_t(0, 0);
    }
    else
    {
        m_DrawData.Vertices[0].position = math::vector_t(0, 0);
        m_DrawData.Vertices[1].position = math::vector_t(m_BoundingBox.w, 0);
        m_DrawData.Vertices[2].position = math::vector_t(m_BoundingBox.w,
                                                         m_BoundingBox.h);
        m_DrawData.Vertices[3].position = math::vector_t(0, m_BoundingBox.h);
    }
}

void zQuad::LoadInvertedVertices()
{
    m_DrawData.Vertices[0].position = math::vector_t(0, 0);
    m_DrawData.Vertices[1].position = math::vector_t(0,
                                        -static_cast<int>(m_BoundingBox.h));
    m_DrawData.Vertices[2].position = math::vector_t(m_BoundingBox.w,
                                        -static_cast<int>(m_BoundingBox.h));
    m_DrawData.Vertices[3].position = math::vector_t(m_BoundingBox.w, 0);
}

void zQuad::LoadRegularTC()
{
    real_t tc_w = 1.0, tc_h = 1.0;

    if(m_rep)
    {
        uint16_t w = m_Material.GetTexture().GetWidth();
        uint16_t h = m_Material.GetTexture().GetHeight();

        tc_w = m_BoundingBox.w / real_t(w);
        tc_h = m_BoundingBox.h / real_t(h);
    }

    m_DrawData.Vertices[0].tc = math::vector_t(0.0, tc_h);
    m_DrawData.Vertices[1].tc = math::vector_t(tc_w, tc_h);
    m_DrawData.Vertices[2].tc = math::vector_t(tc_w, 0.0);
    m_DrawData.Vertices[3].tc = math::vector_t(0.0, 0.0);
}

void zQuad::LoadInvertedTC()
{
    real_t tc_w = 1.0, tc_h = 1.0;

    if(m_rep)
    {
        uint16_t w = m_Material.GetTexture().GetWidth();
        uint16_t h = m_Material.GetTexture().GetHeight();

        tc_w = m_BoundingBox.w / real_t(w);
        tc_h = m_BoundingBox.h / real_t(h);
    }

    m_DrawData.Vertices[0].tc = math::vector_t(0.0, 0.0);
    m_DrawData.Vertices[1].tc = math::vector_t(0.0, tc_h);
    m_DrawData.Vertices[2].tc = math::vector_t(tc_w, tc_h);
    m_DrawData.Vertices[3].tc = math::vector_t(tc_w, 0.0);
}
