template<typename T>
T lerp(const T& Start, const T& End, real_t weight)
{
    clamp<real_t>(weight, 0.f, 1.f);
    return (Start + (End - Start) * weight);
}

template<typename T>
bool in_range(const T& n, const T& minimum, const T& maximum)
{
    return (n >= minimum && n <= maximum);
}
