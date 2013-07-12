#include "Zenderer/CoreGraphics/Drawable.hpp"

using namespace zen;
using gfxcore::CDrawable;

CDrawable::CDrawable() : mp_VAO(nullptr), mp_Material(nullptr),
    mp_MVMatrix(nullptr), m_offset(0), m_internal(false)
{
    m_DrawData.Vertices = nullptr;
    m_DrawData.Indices  = nullptr;
    m_DrawData.icount   =
    m_DrawData.vcount   = 0;
}

CDrawable::CDrawable(const CDrawable& Copy) :
    mp_VAO(nullptr), mp_Material(nullptr),
    mp_MVMatrix(nullptr), m_offset(0),
    m_internal(false)
{
    m_DrawData.vcount   = Copy.m_DrawData.vcount;
    m_DrawData.icount   = Copy.m_DrawData.icount;
    m_DrawData.Vertices = new gfxcore::vertex_t[m_DrawData.vcount];
    m_DrawData.Indices  = new gfxcore::index_t[m_DrawData.icount];

    std::copy(Copy.m_DrawData.Vertices, 
              Copy.m_DrawData.Vertices + m_DrawData.vcount,
              m_DrawData.Vertices);

    std::copy(Copy.m_DrawData.Indices, 
              Copy.m_DrawData.Indices + m_DrawData.icount,
              m_DrawData.Indices);
              
    if(Copy.mp_MVMatrix != nullptr)
        mp_MVMatrix = new math::matrix4x4_t(Copy.mp_MVMatrix);
}

CDrawable::~CDrawable()
{
    if(m_internal) delete mp_VAO;
}

void CDrawable::Move(const math::vector_t& Position)
{
    m_Position = Position;
}

void CDrawable::Move(const real_t x, const real_t y, const real_t z /*= 0.0*/)
{
    m_Position = math::vector_t(x, y, z);
}

void CDrawable::SetColor(const color4f_t& Color)
{
    for(size_t i = 0; i < m_DrawData.vcount; ++i)
    {
        m_DrawData.Vertices[i].color = Color;
    }
}

bool CDrawable::Draw(const bool is_bound /*= false*/)
{
    if(mp_VAO == nullptr && !is_bound)
    {
        // Create a vertex array and load our data.
        mp_VAO = new CVertexArray(GL_STATIC_DRAW);
        mp_VAO->Init();
        m_offset = mp_VAO->AddData(m_DrawData);
        if(!mp_VAO->Offload()) return false;

        // Create our model-view matrix.
        mp_MVMatrix = new math::matrix4x4_t;
       *mp_MVMatrix = math::matrix4x4_t::CreateIdentityMatrix();

        // Our default set of material data, if needed.
        if(!mp_Material) mp_Material = &CRenderer::GetDefaultMaterial();

        // So we can differentiate between a VAO from a `CScene`
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
        
        // Use our effect, or the default?
        gfx::CEffect* pEffect = (mp_Material->GetEffect() == nullptr)
                                ? &CRenderer::GetDefaultEffect()
                                : mp_Material->GetEffect();

        // Use our texture, or the default?
        // We need a default texture because otherwise the color would
        // always be black due to the way the shader works.
        gfxcore::CTexture* pTexture =
            (mp_Material->GetTexture() == nullptr)
            ? const_cast<gfxcore::CTexture*>(&CRenderer::GetDefaultTexture())
            : mp_Material->GetTexture();

        pEffect->Enable();
        pTexture->Bind();

        // All effects have these parameters in the vertex shader.
        if(!pEffect->SetParameter("mv", *mp_MVMatrix) ||
           !pEffect->SetParameter("proj", CRenderer::GetProjectionMatrix()))
        {
            CRenderer::ResetMaterialState();
            return false;
        }
    }

    GL(glDrawElements(GL_TRIANGLES, m_DrawData.icount,
            INDEX_TYPE, (void*)(sizeof(index_t) * m_offset)));

    return !is_bound ?
        (CRenderer::ResetMaterialState() && mp_VAO->Unbind()) :
        false;
}

void CDrawable::LoadIntoVAO(gfxcore::CVertexArray& VAO)
{
    VAO.AddData(m_DrawData);
}
