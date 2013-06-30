/**
 * @file
 *  Zenderer/CoreGraphics/Drawable.hpp - An abstract base class for drawing
 *  primitives.
 *
 * @author      George Kudrayvtsev (halcyon)
 * @version     1.0
 * @copyright   Apache License v2.0
 *  Licensed under the Apache License, Version 2.0 (the "License").         \n
 *  You may not use this file except in compliance with the License.        \n
 *  You may obtain a copy of the License at:
 *  http://www.apache.org/licenses/LICENSE-2.0                              \n
 *  Unless required by applicable law or agreed to in writing, software     \n
 *  distributed under the License is distributed on an "AS IS" BASIS,       \n
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n
 *  See the License for the specific language governing permissions and     \n
 *  limitations under the License.
 *
 * @addtogroup CoreGraphics
 * @{
 **/

#ifndef ZENDERER__CORE_GRAPHICS__DRAWABLE_HPP
#define ZENDERER__CORE_GRAPHICS__DRAWABLE_HPP

#include "Zenderer/Core/Types.hpp"
#include "Zenderer/Math/Math.hpp"
#include "VertexArray.hpp"
#include "Renderer.hpp"
#include "Zenderer/Graphics/Material.hpp"

#include "OpenGL.hpp"

namespace zen
{
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
        CDrawable() : mp_VAO(nullptr),
            mp_Material(nullptr), m_offset(0),
            m_internal(false)
        {
            m_DrawData.Vertices = nullptr;
            m_DrawData.Indices  = nullptr;
            m_DrawData.icount   =
            m_DrawData.vcount   = 0;
        }

        virtual ~CDrawable()
        {
            if(m_internal)
            {
                delete mp_VAO;
            }
        }

        /// Creates initial vertex structure.
        /// @return Reference to itself for easy chaining.
        virtual CDrawable& Create() = 0;

        /**
         * Moves the drawable to a certain location.
         *  This doesn't rely on any vertex data, but rather uses
         *  the model-view matrix to translate the object, thus
         *  there is a default implementation.
         *
         * @param   Position    (x, y, z) coordinates where you want the object
         **/
        void Move(const math::vector_t& Position)
        {
            m_Position = Position;
        }

        /// @overload
        void Move(const real_t x, const real_t y, const real_t z = 0.0)
        {
            m_Position = math::vector_t(x, y, z);
        }

        /**
         * Attaches a material to render on top of the primitive.
         *  This really shouldn't be allowed on simple primitives, but it's
         *  here if you need it. Keep in mind that this will override any
         *  color settings you've made.
         *  Likely this will only work well on quadrilateral primitives due
         *  to difficulties setting texture coordinates on other shapes.
         *
         * @param   pMaterial   The texture you want rendered
         **/
        virtual void AttachMaterial(const gfx::material_t* pMaterial) = 0;

        /// Sets all vertices to have a given color value.
        void SetColor(const color4f_t& Color)
        {
            for(size_t i = 0; i < m_DrawData.vcount; ++i)
            {
                m_DrawData.Vertices[i].color = Color;
            }
        }

        /**
         * Draws the primitive on-screen.
         *  This implements the technique described above. If there is no
         *  "owner" of the primitive (meaning no scene has set the internal
         *  data), it will automatically create a CVertexArray instance,
         *  a model-view matrix, and will use the default shader set.
         *  This data will be re-used time after time on subsequent Draw()
         *  calls, not recreated every time.
         *
         * @param   is_bound    Have we bound things? (VAO, material, etc.)
         *
         * @return  `true` if drawing was successful, `false` otherwise.
         **/
        bool Draw(const bool is_bound = false)
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

                    gfx::CEffect& Shader = CRenderer::GetDefaultEffect();
                    if(!Shader.Enable()) return false;
                    if(!Shader.SetParameter("mv", *mp_MVMatrix) ||
                       !Shader.SetParameter("proj", CRenderer::GetProjectionMatrix()))
                       return false;
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

        inline const math::vector_t& GetPosition() const
        { return m_Position; }

        inline real_t GetX() const { return m_Position.x; }
        inline real_t GetY() const { return m_Position.y; }

        /// For setting things implicitly.
        friend class CSceneManager;

    protected:
        const gfx::material_t* mp_Material;
        math::vector_t      m_Position;
        DrawBatch           m_DrawData;
        bool                m_internal;

    private:
        math::matrix4x4_t*  mp_MVMatrix;
        CVertexArray*       mp_VAO;
        color4f_t           m_Color;
        index_t             m_offset;
    };

}   // namespace gfxcore
}   // namespace zen

#endif // ZENDERER__CORE_GRAPHICS__DRAWABLE_HPP

/** @} **/
