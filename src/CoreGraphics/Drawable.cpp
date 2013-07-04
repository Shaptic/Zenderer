#include "Zenderer/CoreGraphics/Drawable.hpp"

using namespace zen;
using gfxcore::CDrawable;

CDrawable::CDrawable() : mp_VAO(nullptr), mp_Material(nullptr),
    m_offset(0), m_internal(false)
{
    m_DrawData.Vertices = nullptr;
    m_DrawData.Indices  = nullptr;
    m_DrawData.icount   =
    m_DrawData.vcount   = 0;
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
        (*mp_MVMatrix) = math::matrix4x4_t::CreateIdentityMatrix();

        // So we can differentiate between a VAO from a `CScene`
        // and the one we made ourselves.
        m_internal = true;
    }

    // If something isn't previously bound, we bind the VAO
    // and the material. If no material, use global default.
    if(!is_bound)
    {
        if(!mp_VAO->Bind()) return false;
        if(mp_Material == nullptr)
        {
            // Insert our coordinates to transform in the shader.
            // Ignore the Z coordinate because that's only used for depth
            // sorting internally anyway and has no effect on visuals.
            (*mp_MVMatrix)[0][3] = m_Position.x;
            (*mp_MVMatrix)[1][3] = m_Position.y;
            //(*mp_MVMatrix)[2][3] = m_Position.z;

            // Guaranteed to have both shader and texture.
            mp_Material         = &CRenderer::GetDefaultMaterial();
            gfx::CEffect& Shader= *mp_Material->GetEffect();

            mp_Material->Enable();
            if(!Shader.SetParameter("mv", *mp_MVMatrix) ||
               !Shader.SetParameter("proj", CRenderer::GetProjectionMatrix()))
            {
                mp_Material->Disable();
                return false;
            }
        }
    }

    GL(glDrawElements(GL_TRIANGLES, m_DrawData.icount,
            INDEX_TYPE, (void*)(sizeof(index_t) * m_offset)));

    if(!is_bound)
    {
        CRenderer::ResetMaterialState();
        if(!mp_VAO->Unbind()) return false;
    }

    return true;
}

void CDrawable::LoadIntoVAO(gfxcore::CVertexArray& VAO)
{
    VAO.AddData(m_DrawData);
}
