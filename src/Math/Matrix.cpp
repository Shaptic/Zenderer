#include "Zenderer/Math/Matrix.hpp"

using namespace zen;
using namespace math;

matrix4x4_t::matrix4x4_t()
{
    memset(m_values, 0, sizeof(m_values));
}

matrix4x4_t::matrix4x4_t(const real_t values[4][4])
{
    for(size_t i = 0; i < 4; ++i)
        for(size_t j = 0; j < 4; ++j)
            m_values[i][j] = values[i][j];
}

matrix4x4_t::matrix4x4_t(const real_t** ppvalues)
{
    for(size_t i = 0; i < 4; ++i)
        for(size_t j = 0; j < 4; ++j)
            m_values[i][j] = ppvalues[i][j];
}

matrix4x4_t::matrix4x4_t(const matrix4x4_t& Copy)
{
    memcpy(m_values, Copy.GetPointer(), sizeof(m_values));
}

matrix4x4_t& matrix4x4_t::operator=(const matrix4x4_t& Copy)
{
    memcpy(m_values, Copy.GetPointer(), sizeof(m_values));

    return (*this);
}

real_t* matrix4x4_t::operator[](uint8_t index)
{
    clamp<uint8_t>(index, 0, 3);
    return m_values[index];
}

const real_t* const matrix4x4_t::operator[](uint8_t index) const
{
    clamp<uint8_t>(index, 0, 3);
    return m_values[index];
}

matrix4x4_t matrix4x4_t::operator*(matrix4x4_t& Other) const
{
    matrix4x4_t Res;

    for(uint8_t r = 0; r < 4; ++r)
        for(uint8_t c = 0; c < 4; ++c)
            for(uint8_t e = 0; e < 4; ++e)
                Res[r][c] += (m_values[r][e]) * (Other[e][c]);

    return Res;
}

matrix4x4_t matrix4x4_t::CreateIdentityMatrix()
{
    matrix4x4_t Result;
    Result[0][0] = Result[1][1] = Result[2][2] = Result[3][3] = 1.0;
    return Result;
}

const matrix4x4_t& matrix4x4_t::GetIdentityMatrix()
{
    static const matrix4x4_t IDENTITY = matrix4x4_t::CreateIdentityMatrix();
    return IDENTITY;
}
