/**
 * @file
 *  Zenderer/CoreGraphics/VertexArray.hpp - A wrapper class for OpenGL
 *  Vertex Array Objects.
 *
 * @author      George Kudrayvtsev (halcyon)
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
#include "Zenderer/Core/Allocator.hpp"

#include "OpenGL.hpp"

/**
 * Dirty macro to determine the offset of a field within a struct.
 *  Used throughout the CVertexBuffer class to determine the offsets
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
    typedef uint16_t index_t;

    /// Type of indices.
    static const GLenum INDEX_TYPE = GL_UNSIGNED_SHORT;

    /// A collection of drawing data to pass to a CVertexArray.
    struct DrawBatch
    {
        vertex_t*   Vertices;       ///< Vertex array
        index_t*    Indices;        ///< Index array
        size_t      vcount, icount; ///< Vertex / index count
    };

    class ZEN_API CVertexArray : CGLSubsystem
    {
    public:
        CVertexArray(const GLenum vao_type = GL_STATIC_DRAW);
        ~CVertexArray();

        bool Init();
        bool Destroy();

        bool Bind() const;
        bool Unbind() const;

        /**
         * Adds data to the vertex buffer for drawing.
         *  This will append data to the internal buffers for processing.
         *  Nothing is actually offloaded to the GPU; this must be done
         *  with an explicit call to Offload().
         *  It may be desirable to calculate where in the GPU the indices
         *  were placed, possibly in order to properly call `glDrawElements`
         *  later, so this is given as the return value.
         *  When using this value for that purpose, be sure to multiplty by
         *  sizeof(index_t) to get the accurate offset in bytes.
         *
         * @param   D       Vertex and index data to store internally
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

        GLuint GetObjectHandle()    { return m_vao; }
        GLuint GetVBOHandle()       { return m_vbo; }
        GLuint GetIBOHandle()       { return m_ibo; }

        size_t GetVertexCount() const;
        size_t GetIndexCount()  const;

        const vertex_t* const GetVerticesFromGPU()  const;
        const index_t*  const GetIndicesFromGPU()   const;

        bool Offloaded() const;

        bool Draw()
        {
            if(!this->Bind()) return false;
            GL(glDrawElements(GL_TRIANGLES, m_icount, INDEX_TYPE, nullptr));
            return this->Unbind();
        }

    private:
        std::vector<index_t>    m_vaoIndices;
        std::vector<vertex_t>   m_vaoVertices;

        size_t m_icount, m_vcount;

        GLuint m_vao, m_vbo, m_ibo;
        GLenum m_type;
    };
}   // namespace gfxcore
}   // namespace zen

#endif // ZENDERER__CORE_GRAPHICS__VERTEXBUFFER_HPP

/** @} **/
