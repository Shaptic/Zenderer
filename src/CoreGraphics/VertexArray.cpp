#include "Zenderer/CoreGraphics/VertexArray.hpp"

using namespace zen::gfxcore;

zVertexArray::zVertexArray(const GLenum type) :
    zGLSubsystem("Vertex Array"),
    m_icount(0), m_vcount(0),
    m_vao(0), m_vbo(0), m_ibo(0),
    m_type(type)
{
    m_vaoIndices.clear();
    m_vaoVertices.clear();
}

zVertexArray::~zVertexArray()
{
    this->Destroy();
}

bool zVertexArray::Init()
{
    if(m_init) this->Destroy();

    // Bad GL version.
    if(!glGenVertexArrays) return false;

    GL(glGenVertexArrays(1, &m_vao));
    GL(glGenBuffers(1, &m_vbo));
    GL(glGenBuffers(1, &m_ibo));

    ZEN_ASSERT(m_vao != 0);
    ZEN_ASSERT(m_vbo != 0);
    ZEN_ASSERT(m_ibo != 0);

    return (m_init = true);
}

bool zVertexArray::Destroy()
{
    if(!m_init) return true;

    ZEN_ASSERT(m_vao != 0);
    ZEN_ASSERT(m_vbo != 0);
    ZEN_ASSERT(m_ibo != 0);

    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ibo);

    return !(m_init = false);
}

bool zVertexArray::Bind() const
{
    if(!m_init) return false;
    GL(glBindVertexArray(m_vao));
    GL(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
    GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo));
    return true;
}

bool zVertexArray::Unbind() const
{
    if(!m_init) return false;
    GL(glBindVertexArray(0));
    return true;
}

index_t zVertexArray::AddData(const DrawBatch& D)
{
    ZEN_ASSERTM(D.vcount > 0, "no buffer vertices given");
    ZEN_ASSERTM(D.icount > 0, "no buffer indices given");

    // Calculates the offset within the index buffer for the given indices.
    size_t offset = m_vaoIndices.size() + m_icount;

    // We need to account for the fact that the given index buffer assumes that
    // the vertices start at index 0. So we must add the total vertex count,
    // as well as the current (non-loaded) vertex count.
    m_vaoIndices.reserve(m_vaoIndices.size() + D.icount);
    for(size_t i = 0; i < D.icount; ++i)
        m_vaoIndices.push_back(D.Indices[i] + m_vcount + m_vaoVertices.size());

    m_vaoVertices.reserve(m_vaoVertices.size() + D.vcount);
    m_vaoVertices.insert(m_vaoVertices.end(), D.Vertices, D.Vertices + D.vcount);

    return offset;
}

/// @see    http://stackoverflow.com/questions/8923174/opengl-vao-best-practices
bool zVertexArray::Offload()
{
    if(!this->Bind())       return false;
    if(this->Offloaded())   return false;

    // Check if there's existing data on the buffers.
    GLint bsize = 0;
    GL(glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &bsize));

    // There is!
    if(bsize > 0)
    {
        // Copy existing buffer data from GPU to local buffer.
        const vertex_t* const data = this->GetVerticesFromGPU();
        const size_t size = bsize / sizeof(vertex_t);

        m_vaoVertices.reserve(m_vaoVertices.size() + size);
        for(int i = size - 1; i >= 0; --i)
            m_vaoVertices.insert(m_vaoVertices.begin(), data[i]);

        m_vcount = 0;
        GL(glUnmapBuffer(GL_ARRAY_BUFFER));
    }

    // Allocate enough GPU space for all vertex data, new and old, and pass
    // all of the data directly to it.
    GL(glBufferData(GL_ARRAY_BUFFER,
                    sizeof(vertex_t) * m_vaoVertices.size(),
                    &m_vaoVertices[0], m_type));

    // Repeat process for index buffer.
    bsize = 0;
    GL(glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &bsize));

    if(bsize > 0)
    {
        // Copy from GPU to local buffer.
        const index_t* const data = this->GetIndicesFromGPU();
        const size_t size = bsize / sizeof(index_t);

        m_vaoIndices.reserve(m_vaoIndices.size() + size);
        for(int i = size - 1; i >= 0; --i)
            m_vaoIndices.insert(m_vaoIndices.begin(), data[i]);

        m_icount = 0;
        GL(glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER));
    }

    // Allocate enough GPU space for all index data, new and old, and pass
    // all of the data directly to it.
    GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER,                // IBO
                    sizeof(index_t) * m_vaoIndices.size(),  // Size
                    &m_vaoIndices[0],                       // Initial data
                    m_type));                               // Access type

    // Vertices are arranged in memory like so:
    //
    // [ x, y, z, t, s, r, r, g, b, a ]
    //
    // (see the definition of vertex_t in Zenderer/CoreGraphics/OpenGL.hpp)
    //
    // Specify vertex position arrangement.
    // According to the diagram shown above, the vertex position
    // would start at index 0.
    GL(glVertexAttribPointer(0,                 /* Attribute index  */
                             3,                 /* Number of values */
                             GL_FLOAT,          /* Type of value    */
                             GL_FALSE,          /* Normalized?      */
                             sizeof(vertex_t),  /* Size of field    */
        VBO_OFFSET(0, vertex_t, position)));    /* Size of offset   */

    // Enable shader attribute 0 (position)
    GL(glEnableVertexAttribArray(0));

    // Specify texture coordinate position arrangement.
    // According to the diagram, texture coordinates start at index 3.
    GL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
                             VBO_OFFSET(0, vertex_t, tc)));
    GL(glEnableVertexAttribArray(1));

    // Specify the color arrangement, starting at index 4.
    GL(glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
                             VBO_OFFSET(0, vertex_t, color)));
    GL(glEnableVertexAttribArray(2));

    // We do not unbind our buffers as they stay attached to the VAO.

    // We're done, clean up buffers.
    m_vcount += m_vaoVertices.size();
    m_icount += m_vaoIndices.size();

    m_vaoVertices.clear();
    m_vaoIndices.clear();

    return this->Unbind();
}

bool zVertexArray::Clear()
{
    if(!this->Bind()) return false;
    GL(glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW));
    GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW));

    m_icount = m_vcount = 0;
    m_vaoIndices.clear();
    m_vaoVertices.clear();

    return this->Unbind();
}

const vertex_t* const zVertexArray::GetVerticesFromGPU() const
{
    return (vertex_t*)GL(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY));
}

const index_t* const zVertexArray::GetIndicesFromGPU() const
{
    return (index_t*)GL(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY));
}

size_t zVertexArray::GetVertexCount() const
{
    return m_vcount;
}

size_t zVertexArray::GetIndexCount() const
{
    return m_icount;
}

bool zVertexArray::Offloaded() const
{
    return (m_vaoIndices.empty() && m_vaoVertices.empty());
}
