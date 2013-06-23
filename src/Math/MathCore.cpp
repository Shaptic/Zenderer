#include "Zenderer/Math/MathCore.hpp"

using namespace zen;

real_t math::rad(const real_t degrees)
{
    return (degrees * math::PI / 180.0);
}

real_t math::deg(const real_t radians)
{
    return (radians * 180.0 / math::PI);
}

bool math::compf(const real_t a, const real_t b, const real_t threshold)
{
    return (a + threshold > b && a - threshold < b);
}
