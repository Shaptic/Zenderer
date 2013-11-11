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

real_t math::distance(const real_t x1, const real_t y1,
                      const real_t x2, const real_t y2,
                      const bool do_sqrt)
{
    real_t pyth = (x1 - x2) * (x1 - x2) +
                  (y1 - y2) * (y1 - y2);
    return !do_sqrt ? pyth : std::sqrt(pyth);
}
