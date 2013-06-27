#ifndef ZENDERER__CORE_GRAPHICS__DRAWABLE_HPP
#define ZENDERER__CORE_GRAPHICS__DRAWABLE_HPP

#include "Zenderer/Core/Types.hpp"
#include "Zenderer/Math/Math.hpp"
#include "Zenderer/Assets/Texture.hpp"

#include "OpenGL.hpp"

namespace zen
namespace gfxcore
{
    /**
     * An abstract base class for all drawable primitives.
     *  There are two ways to create renderable objects in @a Zenderer.
     *
     *  Firstly is through the `zen::gfx::CScene` class and its respective
     *  `AddPrimitive()` or `AddEntity()` methods. This will attach an 
     *  internal `zen::gfxcore::CVertexArray` object that stores scene
     *  geometry. The scene itself takes care of the actual drawing, but
     *  the user can still call the `Drawable::Draw()` method if they
     *  want to do it again, post-render. 
     *
     *  The other way is to simply create a drawable object instance and
     *  call `Draw()` on it. This will implicitly create a vertex array
     *  object internally the first time, and will just use it every
     *  subsequent call. Keep in mind that this object now can **NOT** 
     *  be added to a scene. This functionality may be supported in future
     *  revisions.
     **/
    class ZEN_API CDrawable
    {
    public:
        CDrawable();
        virtual ~CDrawable();
        
        /// Creates initial vertex structure.
        virtual void Create() = 0;
        
        /**
         * Moves the drawable to a certain location.
         *  This doesn't rely on any vertex data, but rather uses
         *  the model-view matrix to translate the object, thus
         *  there is a default implementation.
         *
         * @param   Position    (x, y, z) coordinates where you want the object
         **/
        virtual void Move(const math::vector_t& Position);
        
        /**
         * Attaches a texture to render on top of the primitive.
         *  This really shouldn't be allowed on simple primitives, but it's
         *  here if you need it. Keep in mind that this will override any
         *  color settings you've made.
         *  Likely this will only work well on quadrilateral primitives due
         *  to difficulties setting texture coordinates on other shapes.
         *
         * @param   pTexture    The texture you want rendered
         **/
        virtual void AttachTexture(const gfx::CTexture* pTexture) = 0;
        
        /// Sets all vertex colors to the given value.
        virtual void SetColor(const color4f_t& Color);
        
        /// For setting things implicitly.
        friend class CSceneManager;
    private:
        CVertexArray*  mp_VAO;
        gfx::CTexture* mp_Texture;
        
        math::vector_t m_Position;
        
        color4f_t      m_Color;
        vertex_t*      mp_vertexList;
        index_t*       mp_indexList;
        
        size_t         m_vcount, m_icount;
    };

    void CQuad::Create()
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
