/**
 * @file
 *  Zenderer/CoreGraphics/VertexArray.hpp - A wrapper class for OpenGL
 *  Vertex Array Objects.
 *
 * @author      George (@_Shaptic)
 * @version     1.1
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

#ifndef ZENDERER__CORE_GRAPHICS__VERTEXBUFFER_HPP
#define ZENDERER__CORE_GRAPHICS__VERTEXBUFFER_HPP

#include <vector>

#include "Zenderer/Core/Types.hpp"
#include "OpenGL.hpp"

/**
 * Dirty macro to determine the offset of a field within a struct.
 *  Used throughout the `zVertexBuffer` class to determine the offsets
 *  of data in the GPU buffers.
 **/
#define VBO_OFFSET(c, object, field)\
    (void*)((c * sizeof(object)) +  \
            (long int)&(((object*)nullptr)->field))

namespace zen
{
namespace gfxcore
{
    /// Shortcut for easy modification.
    using index_t = uint16_t;

    /// Type of indices.
    static const GLenum INDEX_TYPE = GL_UNSIGNED_SHORT;

    /// A collection of drawing data to pass to a `zVertexArray`.
    struct DrawBatch
    {
        vertex_t*   Vertices;       ///< Vertex array
        index_t*    Indices;        ///< Index array
        size_t      vcount, icount; ///< Vertex / index count
    };

    /// A wrapper for OpenGL Vertex Array Objects.
    class ZEN_API zVertexArray : public zGLSubsystem
    {
    public:
        zVertexArray(const GLenum vao_type = GL_STATIC_DRAW);
        ~zVertexArray();

        bool Init();
        bool Destroy();

        bool Bind() const;
        bool Unbind() const;

        /**
         * Adds data to the vertex buffer for drawing.
         *  This will append data to the internal buffers for processing.
         *  Nothing is actually offloaded to the GPU; this must be done
         *  with an explicit call to `Offload()`.
         *  It may be desirable to calculate where in the GPU the indices
         *  were placed, possibly in order to properly call `glDrawElements()`
         *  later, so this is given as the return value.
         *  When using this value for that purpose, be sure to multiply by
         *  `sizeof(index_t)` to get the accurate offset in bytes.
         *
         * @param   D   Vertex and index data to store internally
         *
         * @return  Internal index buffer offset value.
         **/
        index_t AddData(const DrawBatch& D);

        /**
         * Offloads local vertex data to the GPU.
         *  If there is existing buffer data on the GPU, this will copy the
         *  buffer locally and append any data that was added recently
         *  (via AddData()) to the end of the buffer. Then it will be
         *  sent off to the GPU again.
         *
         *  This will return `false` only in a release build, in debug builds
         *  any failed OpenGL operations will close the program.
         *
         * @return  `true` if everything was offloaded without errors;
         *          `false` otherwise.
         *
         * @see     zen::gfxcore::vertex_t
         * @see     Zenderer/CoreGraphics/OpenGL.hpp
         **/
        bool Offload();

        /// Deletes all vertex and index data from the GPU and locally.
        bool Clear();

        GLuint GetObjectHandle() const  { return m_vao; }
        GLuint GetVBOHandle()    const  { return m_vbo; }
        GLuint GetIBOHandle()    const  { return m_ibo; }

        size_t GetVertexCount() const;
        size_t GetIndexCount()  const;

        /**
         * This retrieves raw vertex data directly from VRAM.
         *
         * @return  An array of `vertex_t` values. The size of this array can
         *          be determined by `GetVertexCount()`.
         *
         * @warning You *MUST* call `glUnmapBuffer(GL_ARRAY_BUFFER)` after
         *          calling this method, which is why it is ill-advised to mess
         *          with this.
         **/
        const vertex_t* const GetVerticesFromGPU() const;

        /**
         * This retrieves raw index data directly from VRAM.
         *
         * @return  An array of `index _t` values. The size of this array can
         *          be determined by `GetIndexCount()`.
         *
         * @warning You *MUST* call `glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER)`
         *          after calling this method, which is why it is ill-advised
         *          to mess with this.
         **/
        const index_t* const GetIndicesFromGPU() const;

        /// Tells the caller whether or not all data has been sent to VRAM.
        bool Offloaded() const;

        inline bool Draw() const
        {
            if(!this->Bind()) return false;
            GL(glDrawElements(GL_TRIANGLES, m_icount, INDEX_TYPE, nullptr));
            return this->Unbind();
        }

    private:
        std::vector<index_t>    m_vaoIndices;
        std::vector<vertex_t>   m_vaoVertices;

        size_t m_icount, m_vcount;
        size_t m_icapacity, m_vcapacity;

        GLuint m_vao, m_vbo, m_ibo;
        GLenum m_type;
    };
}   // namespace gfxcore
}   // namespace zen

#endif // ZENDERER__CORE_GRAPHICS__VERTEXBUFFER_HPP

/**
 * @class zen::gfxcore::zVertexArray
 * @details
 *  This manages offloading raw, low-level draw data to the GPU for
 *  efficient access and local memory consumption. It's used internally
 *  by the scene wrapper to store scene geometry.
 *
 * @todo    More documentation.
 *
 * @example VertexArray
 * @section vao_draw    Raw Vertex Drawing
 *  Sometimes (for whatever insane reason), the need arises to write directly
 *  to the GPU in order to draw raw vertices to the screen. This example explains
 *  how that should be done.
 *
 * @subsection db   The `DrawBatch` structure
 *  This is a simple structure that stores vertex and index data for offloading
 *  to the GPU. Since @a Zenderer uses an indexed rendering approach, it's
 *  necessary to supply indices to the corresponding vertices you wish to use.
 *  I won't go into serious detail, but basically the indices act as array
 *  indices for the vertex buffer. Like so:
 *
 *  @code
 *  // Imagine the GPU vertex buffer is like this:
 *  vertex_t triangle[] = {
 *      { (0, 0, 0),    (0, 0), (1, 1, 1, 1) },    // A simple white triangle
 *      { (100, 0, 0),  (1, 0), (1, 1, 1, 1) },
 *      { (100, 100, 0),(1, 1), (1, 1, 1, 1) }
 *  };
 *
 *  // The indices work like so:
 *  index_t indices[] = {0, 1, 2};
 *
 *  // So indices[0] will draw triangle[0], and so on.
 *  @endcode
 *
 *  To draw vertex data, you need vertices and indices. You can, of course, just
 *  specify 6 vertices to get a quad (two triangles), *or* you can specify 4
 *  vertices and re-use two of them with indices (since they are shared). To get
 *  to the point, fill up the `DrawBatch` struct with your data, and then you
 *  can pass it on to the `zVertexArray` instance for offloading.
 *
 *  @code
 *  using namespace zen::gfxcore;
 *  using namespace zen::math;
 *
 *  vertex_t* vertices = new vertex_t[4];
 *  index_t* indices = new index_t[6];
 *
 *  // Create a 32x32 red quad.
 *  vertices[1].position = vector_t(32, 0);
 *  vertices[2].position = vector_t(32, 32);
 *  vertices[3].position = vector_t(0, 32);
 *
 *  for(size_t i = 0; i < 4; ++i)
 *      vertices[i].color = zen::color4f_t(1, 0, 0, 1);
 *
 *  indices[0] = 0;
 *  indices[1] = 1;
 *  indices[2] = 3;
 *  indices[3] = 3;
 *  indices[4] = 1;
 *  indices[5] = 2;
 *
 *  // You can also skip the dynamic allocation for the indices
 *  // if you just want a quad and use zen::zQuad::GetIndexArray().
 *  // const index_t& indices = zen::zQuad::GetIndexArray();
 *  // but then you must be careful not to delete[] them :)
 *
 *  DrawBatch D = {vertices, 4, indices, 6};
 *
 *  // ... to be continued.
 *  @endcode
 *
 *  @subsection vao     Offloading to the GPU and Drawing
 *  Now we have our data to draw on the screen. Here's how that works:
 *
 *  @code
 *  // ... continued from the previous section
 *
 *  // Create the GPU buffers
 *  zVertexArray VAO();
 *  VAO.Init();
 *  VAO.AddData(D);
 *  VAO.Offload();
 *
 *  // Data is on the GPU now, the DrawBatch can be safely deleted.
 *  delete[] vertices;
 *  delete[] indices;
 *  vertices = indices = nullptr;
 *
 *  // Drawing can be done in a loop like so (assuming a window exists)
 *  zRenderer::GetDefaultMaterial().Enable();
 *  while(Window.IsOpen())
 *  {
 *      VAO.Draw();
 *      Window.Update();
 *  }
 *  @endcode
 *
 *  And there you have it, a 32x32 quad drawn directly from the GPU on
 *  the screen.
 **/

/** @} **/
