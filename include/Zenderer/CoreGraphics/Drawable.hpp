#ifndef ZENDERER__CORE_GRAPHICS__DRAWABLE_HPP
#define ZENDERER__CORE_GRAPHICS__DRAWABLE_HPP

#include "Zenderer/Core/Types.hpp"
#include "Zenderer/Math/Math.hpp"
#include "Zenderer/Assets/Texture.hpp"

#include "OpenGL.hpp"

namespace zen
namespace gfxcore
{
    class ZEN_API CDrawable
    {
    public:
        CDrawable();
        virtual ~CDrawable();
        
        virtual void Create(const math::vector_t& Position,
                            const color4f_t& Color) = 0;
                            
        inline void Recreate() { this->Create(m_Position, m_Color); }
        
        virtual void AttachTexture(const asset::CTexture* pTexture) = 0;
        
        void SetColor(const color4f_t& Color);
        void SetColor(const real_t r, const real_t g,
                      const real_t b, const real_t a);
                      
        void SetPosition(const math::vector_t& Position);
        void SetPosition(const real_t x, const real_t y);
        
        friend class CSceneManager;
        
    private:
        asset::CTexture*    mp_Texture;
        math::vector_t      m_Position;
        
        color4f_t           m_Color;
        vertex_t*           mp_vertexList;
        index_t*            mp_indexList;
        
        size_t              m_vcount, m_icount;
    };

    void CQuad::Create(const math::vector_t& Position, const color4f_t& Color)
    {
        if(mp_vertexList == nullptr)
        {
            mp_vertexList = new vertex_t[4];
            m_vcount = 4;
        }
        
        mp_vertexList[0].position = Position;
        
        mp_vertexList[1].position = math::vector_t(Position.x + m_Size.w,
                                                   Position.y);
                                                   
        mp_vertexList[2].position = math::vector_t(Position.x + m_Size.w,
                                                   Position.y + m_Size.h);

        mp_vertexList[3].position = math::vector_t(Position.x,
                                                   Position.y + m_Size.h);
                                                   
        mp_vertexList[0].color = mp_vertexList[1].color =
        mp_vertexList[2].color = mp_vertexList[3].color = Color;
        
        mp_vertexList[0].tc = math::vector_t(0.0, 0.0);
        mp_vertexList[1].tc = math::vector_t(1.0, 0.0);
        mp_vertexList[2].tc = math::vector_t(1.0, 1.0);
        mp_vertexList[3].tc = math::vector_t(0.0, 1.0);
        
        if(mp_indexList == nullptr)
        {
            mp_indexList = new index_t[6];
            m_icount = 6;
        }
        
        mp_indexList[0] = 0;
        mp_indexList[1] = 1;
        mp_indexList[2] = 3;
        mp_indexList[3] = 3;
        mp_indexList[4] = 1;
        mp_indexList[5] = 2;
        
        m_Position  = Position;
        m_Color     = Color;
    }
}   // namespace gfxcore
}   // namespace zen

#endif // ZENDERER__CORE_GRAPHICS__DRAWABLE_HPP
/** @} **/
