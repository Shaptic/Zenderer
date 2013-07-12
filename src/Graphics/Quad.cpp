#include "Zenderer/Graphics/Quad.hpp"

using namespace zen;
using gfx::CQuad;

CQuad::CQuad(const math::rect_t& Size) :
    m_Size(Size), m_inv(false), m_rep(false)
{}

CQuad::CQuad(const uint16_t w, const uint16_t h) : 
    m_Size(0, 0, w, h), m_inv(false), m_rep(false)
{}

CQuad::CQuad(const CQuad& Copy) : CDrawable(Copy)
{
    m_Size = math::rect_t(0, 0, Copy.m_Size.w, Copy.m_Size.h);
    m_inv = Copy.m_inv;
    m_rep = Copy.m_rep;
}

CQuad::~CQuad()
{
    if(m_DrawData.Vertices != nullptr) delete m_DrawData.Vertices;
    if(m_DrawData.Indices  != nullptr) delete m_DrawData.Indices;
}

gfxcore::CDrawable& CQuad::Create()
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

    if(mp_Material == nullptr) 
        this->AttachMaterial(&gfxcore::CRenderer::GetDefaultMaterial());

    return (*this);
}

void CQuad::AttachMaterial(gfx::CMaterial* pMaterial)
{
    mp_Material = pMaterial;
}

void CQuad::Resize(const math::vectoru16_t& Size)
{
    this->Resize(Size.x, Size.y);
}

void CQuad::Resize(const uint16_t w, const uint16_t h)
{
    // Offload()'ed already.
    if(!this->IsModifiable()) return;

    m_Size.x = w;
    m_Size.y = h;
}

void CQuad::SetInverted(const bool flag)
{
    m_inv = flag;
}

void CQuad::SetRepeating(const bool flag)
{
    m_rep = flag;
}

void CQuad::LoadRegularVertices()
{
    m_DrawData.Vertices[0].position = math::vector_t();
    m_DrawData.Vertices[1].position = math::vector_t(m_Size.x, 0.0);
    m_DrawData.Vertices[2].position = math::vector_t(m_Size.x, m_Size.y);
    m_DrawData.Vertices[3].position = math::vector_t(0.0, m_Size.y);
}

void CQuad::LoadInvertedVertices()
{
    // We cast to int since -h and -w is invalid for unsigned integers.
    m_DrawData.Vertices[0].position = math::vector_t(0, 0);
    m_DrawData.Vertices[1].position = math::vector_t(0, -(int)m_Size.y);
    m_DrawData.Vertices[2].position = math::vector_t(m_Size.x, -(int)m_Size.y);
    m_DrawData.Vertices[3].position = math::vector_t(m_Size.x, 0);
}

void CQuad::LoadRegularTC()
{
    real_t tc_w = 1.0, tc_h = 1.0;
    
    if(m_rep && mp_Material != nullptr && mp_Material->GetTexture() != nullptr)
    {
        uint16_t w = mp_Material->GetTexture()->GetWidth();
        uint16_t h = mp_Material->GetTexture()->GetHeight();
        
        tc_w = m_Size.x / real_t(w);
        tc_h = m_Size.y / real_t(h);
    }
    
    m_DrawData.Vertices[0].tc = math::vector_t(0.0, 0.0);
    m_DrawData.Vertices[1].tc = math::vector_t(tc_w, 0.0);
    m_DrawData.Vertices[2].tc = math::vector_t(tc_w, tc_h);
    m_DrawData.Vertices[3].tc = math::vector_t(0.0, tc_h);
}

void CQuad::LoadInvertedTC()
{
    real_t tc_w = 1.0, tc_h = 1.0;
    
    if(m_rep && mp_Material != nullptr && mp_Material->GetTexture() != nullptr)
    {
        uint16_t w = mp_Material->GetTexture()->GetWidth();
        uint16_t h = mp_Material->GetTexture()->GetHeight();
        
        tc_w = m_Size.x / real_t(w);
        tc_h = m_Size.y / real_t(h);
    }
    
    m_DrawData.Vertices[0].tc = math::vector_t(0.0, 0.0);
    m_DrawData.Vertices[1].tc = math::vector_t(0.0, tc_h);
    m_DrawData.Vertices[2].tc = math::vector_t(tc_w, tc_h);
    m_DrawData.Vertices[3].tc = math::vector_t(tc_w, 0.0);
}
