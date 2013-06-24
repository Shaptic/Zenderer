vectorf_t& vectorf_t::operator=(const vectorf_t& Copy)
{
    x = Copy.x;
    y = Copy.y;
    z = Copy.z;

    return *this;
}

bool vectorf_t::operator==(const vectorf_t& Other) const
{
    return compf(x, Other.x) && compf(y, Other.y) && compf(z, Other.z);
}

bool vectorf_t::operator!=(const vectorf_t& Other) const
{
    return !((*this) == Other);
}

vectorf_t vectorf_t::operator^(const vectorf_t& Other) const
{
    return vectorf_t(y * Other.z - z * Other.y,
                     x * Other.z - z * Other.x,
                     x * Other.y - y * Other.x);
}

real_t vectorf_t::operator*(const vectorf_t& Other) const
{
    return x * Other.x + y * Other.y + z * Other.z;
}

vectorf_t vectorf_t::operator*(const real_t scalar) const
{
    return vectorf_t(x * scalar, y * scalar, z * scalar);
}

vectorf_t vectorf_t::operator/(const real_t scalar) const
{
    return (*this) * (1 / scalar);
}

vectorf_t vectorf_t::operator+(const vectorf_t& Other) const
{
    return vectorf_t(x + Other.x, y + Other.y, z + Other.z);
}

vectorf_t vectorf_t::operator+(const real_t value) const
{
    return vectorf_t(x + value, y + value, z + value);
}

vectorf_t vectorf_t::operator-(const vectorf_t& Other) const
{
    return vectorf_t(x - Other.x, y - Other.y, z - Other.z);
}

void vectorf_t::Normalize()
{
    real_t mag = this->Magnitude();

    if(!compf(mag, 0.0))
    {
        x /= mag;
        y /= mag;
        z /= mag;
    }
}

void vectorf_t::Rotate(const real_t radians)
{
    real_t c = cos(radians);
    real_t s = sin(radians);
    real_t old_x = x;

    x = x * c - y * s;
    y = old_x * s + y * c;
}

real_t vectorf_t::Cross2D(const vectorf_t& Other) const
{
    return ((*this) ^ Other).Magnitude();
}

vectorf_t vectorf_t::GetNormalized() const
{
    real_t mag = this->Magnitude();
    return vectorf_t(x / mag, y / mag);
}

real_t vectorf_t::Magnitude() const
{
    return sqrt((*this) * (*this));
}

std::ostream& operator<<(std::ostream& out, const vectorf_t& V)
{
    out.setf(std::ios::fixed, std::ios::floatfield);
    out.precision(2);
    out << "<" << V.x << ", " << V.y << ", " << V.z << ")";
    return out;
}
