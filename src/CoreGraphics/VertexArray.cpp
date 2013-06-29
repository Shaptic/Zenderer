#include "Zenderer/CoreGraphics/VertexArray.hpp"

using namespace zen::gfxcore;

CVertexArray::CVertexArray(const GLenum type) :
    CGLSubsystem("Vertex Array"),
    m_icount(0), m_vcount(0),
    m_vao(0), m_vbo(0), m_ibo(0),
    m_type(type)
{
    m_vaoIndices.clear();
    m_vaoVertices.clear();
}

CVertexArray::~CVertexArray()
{
    this->Destroy();
}

bool CVertexArray::Init()
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

bool CVertexArray::Destroy()
{
    if(!m_init) return true;

    ZEN_ASSERT(m_vao != 0);
    ZEN_ASSERT(m_vbo != 0);
    ZEN_ASSERT(m_ibo != 0);

    glDeleteVertexArrays(1, &m_vao);
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_ibo);

    return true;
}

bool CVertexArray::Bind()
{
    if(!m_init) return false;

    GL(glBindVertexArray(m_vao));
    GL(glEnableVertexAttribArray(0));       // Enable shader attribute 0
    GL(glEnableVertexAttribArray(1));
    GL(glEnableVertexAttribArray(2));

    GL(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
    GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo));

    return true;
}

bool CVertexArray::Unbind()
{
    if(!m_init) return false;

    GL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
    GL(glBindVertexArray(0));

    return true;
}

index_t CVertexArray::AddData(const DrawBatch& D)
{
    ZEN_ASSERTM(D.vcount > 0, "no buffer vertices given");
    ZEN_ASSERTM(D.icount > 0, "no buffer indices given");

    m_vaoVertices.resize(m_vaoVertices.size() + D.vcount);
    for(size_t v = 0; v < D.vcount; ++v)
        m_vaoVertices.push_back(D.Vertices[v]);

    size_t offset = m_vaoIndices.size();

    m_vaoIndices.resize(m_vaoIndices.size() + D.icount);
    for(size_t i = 0; i < D.icount; ++i)
        m_vaoIndices.push_back(D.Indices[i]);

    return offset + m_icount;
}

bool CVertexArray::Offload()
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
        vertex_t* tmp  = new vertex_t[bsize / sizeof(vertex_t)];
        memcpy(tmp, data, bsize);
        GL(glUnmapBuffer(GL_ARRAY_BUFFER));

        // Allocate enough GPU space for all vertex data, new and old.
        // Pass the old data directly to it.
        GL(glBufferData(GL_ARRAY_BUFFER,
                        bsize + (sizeof(vertex_t) * m_vaoVertices.size()),
                        tmp, m_type));

        // Pass the latest vertex data at the end of the existing data.
        GL(glBufferSubData(GL_ARRAY_BUFFER, bsize,
                           sizeof(vertex_t) * m_vaoVertices.size(),
                           &m_vaoVertices[0]));
    }
    // No existing buffer or vertices.
    else
    {
        // Allocate enough space for all vertices on GPU.
        GL(glBufferData(GL_ARRAY_BUFFER,
                        sizeof(vertex_t) * m_vaoVertices.size(),
                        &m_vaoVertices[0], m_type));
    }

    // Repeat process for index buffer.
    bsize = 0;
    GL(glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &bsize));

    if(bsize > 0)
    {
        // Copy from GPU to local buffer.
        const index_t* const data = this->GetIndicesFromGPU();
        index_t* tmp  = new index_t[bsize / sizeof(index_t)];
        memcpy(tmp, data, bsize);
        GL(glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER));

        // Allocate enough GPU space for all vertex data, new and old.
        // Pass the old data directly to it.
        GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER,                        // IBO
                        bsize + (sizeof(index_t) * m_vaoIndices.size()),// Size
                        tmp,       // Initial data
                        m_type));  // Access type

        // Pass the latest vertex data at the end of the existing data.
        GL(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, bsize,
                           sizeof(index_t) * m_vaoIndices.size(),
                           &m_vaoIndices[0]));
    }
    else
    {
        // No existing data, so we just write new stuff to the buffer.
        GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                        sizeof(index_t) * m_vaoIndices.size(),
                        &m_vaoIndices[0], m_type));
    }

    // Vertices are arranged in memory like so:
    //
    // [ x, y, z, t, s, r, r, g, b, a ]
    //
    // (see the definition of vertex_t in Zenderer/CoreGraphics/OpenGL.hpp)
    //
    // Specify vertex position arrangement.
    // According to the diagram shown above, the vertex position
    // would start at index 0.
    GL(glVertexAttribPointer(0,             /* Attribute index  */
                          3,                /* Number of values */
                          GL_FLOAT,         /* Type of value    */
                          GL_FALSE,         /* Normalized?      */
                          sizeof(vertex_t), /* Size of field    */
        VBO_OFFSET(0, vertex_t, position)));/* Size of offset   */

    // Specify texture coordinate position arrangement.
    // According to the diagram, texture coordinates
    // start at index 3.
    GL(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
                             VBO_OFFSET(0, vertex_t, tc)));

    // Specify the color arrangement, starting at index 4.
    GL(glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vertex_t),
                             VBO_OFFSET(0, vertex_t, color)));

    // We're done, clean up buffers.
    m_vcount += m_vaoVertices.size();
    m_icount += m_vaoIndices.size();

    m_vaoVertices.clear();
    m_vaoIndices.clear();

    return this->Unbind();
}

const vertex_t* const CVertexArray::GetVerticesFromGPU() const
{
    return (vertex_t*)GL(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY));
}

const index_t* const CVertexArray::GetIndicesFromGPU() const
{
    return (index_t*)GL(glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY));
}

size_t CVertexArray::GetVertexCount() const
{
    return m_vcount;
}

size_t CVertexArray::GetIndexCount() const
{
    return m_icount;
}

bool CVertexArray::Offloaded() const
{
    return (m_vaoIndices.size() == 0 && m_vaoVertices.size() == 0);
}
