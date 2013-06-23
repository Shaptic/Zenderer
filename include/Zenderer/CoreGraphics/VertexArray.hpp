/**
 * @file
 *  Zenderer/GraphicsCore/VertexArray.hpp - A wrapper class for OpenGL
 *  Vertex Array Objects.
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
 * @addtogroup GraphicsCore
 * @{
 **/

#ifndef ZENDERER__GRAPHICS_CORE__VERTEXBUFFER_HPP
#define ZENDERER__GRAPHICS_CORE__VERTEXBUFFER_HPP

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
    static const GLenum INDEX_TYPE = GL_UNSIGNED_BYTE;

    struct DrawBatch
    {
        vertex_t*   Vertices;
        index_t*    Indices;
        size_t      vcount, icount;
    };

    class ZEN_API CVertexArray : CGLSubsystem
    {
    public:
        CVertexArray(const GLenum vao_type = GL_STATIC_DRAW);
        ~CVertexArray();

        bool Init();
        bool Destroy();

        bool Bind() { return false; }
        bool Unbind() { return false; }

        bool AddData(const DrawBatch& D);
        
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

        bool Offloaded()        const;

    private:
        std::vector<index_t>    m_vaoIndices;
        std::vector<vertex_t>   m_vaoVertices;

        size_t m_icount, m_vcount;

        GLuint m_vao, m_vbo, m_ibo;
        GLenum m_type;
    };
}   // namespace gfxcore
}   // namespace zen

#endif // ZENDERER__GRAPHICS_CORE__VERTEXBUFFER_HPP

/** @} **/
