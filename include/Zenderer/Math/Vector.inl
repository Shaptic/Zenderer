template<typename T>
template<typename U>
zVector<T>& zVector<T>::operator=(const zVector<U>& Copy)
{
    x = Copy.x;
    y = Copy.y;
    z = Copy.z;

    return *this;
}

template<typename T>
template<typename U>
bool zVector<T>::operator==(const zVector<U>& Other) const
{
    return compf(x, Other.x) && compf(y, Other.y) && compf(z, Other.z);
}

template<typename T>
template<typename U>
bool zVector<T>::operator!=(const zVector<U>& Other) const
{
    return !((*this) == Other);
}

template<typename T>
template<typename U>
zVector<T> zVector<T>::operator^(const zVector<U>& Other) const
{
    return zVector<T>(y * Other.z - z * Other.y,
                      x * Other.z - z * Other.x,
                      x * Other.y - y * Other.x);
}

template<typename T>
template<typename U>
real_t zVector<T>::operator*(const zVector<U>& Other) const
{
    return x * Other.x + y * Other.y + z * Other.z;
}

template<typename T>
zVector<T> zVector<T>::operator*(const real_t scalar) const
{
    return zVector<T>(x * scalar, y * scalar, z * scalar);
}

template<typename T>
zVector<T> zVector<T>::operator/(const real_t scalar) const
{
    return (*this) * (1 / scalar);
}

template<typename T>
template<typename U>
zVector<T> zVector<T>::operator+(const zVector<U>& Other) const
{
    return zVector<T>(x + Other.x, y + Other.y, z + Other.z);
}

template<typename T>
zVector<T> zVector<T>::operator+(const real_t value) const
{
    return zVector<T>(x + value, y + value, z + value);
}

template<typename T>
template<typename U>
zVector<T> zVector<T>::operator-(const zVector<U>& Other) const
{
    return zVector<T>(x - Other.x, y - Other.y, z - Other.z);
}

template<typename T>
void zVector<T>::Normalize()
{
    real_t mag = this->Magnitude();

    if(!compf(mag, 0.0))
    {
        x /= mag;
        y /= mag;
        z /= mag;
    }
}

template<typename T>
void zVector<T>::Rotate(const real_t radians)
{
    real_t c = cos(radians);
    real_t s = sin(radians);
    real_t old_x = x;

    x = x * c - y * s;
    y = old_x * s + y * c;
}

template<typename T>
template<typename U>
real_t zVector<T>::Cross2D(const zVector<U>& Other) const
{
    return x * Other.y - y * Other.x;
}

template<typename T>
zVector<T> zVector<T>::GetNormalized() const
{
    real_t mag = this->Magnitude();
    return zVector<T>(x / mag, y / mag);
}

template<typename T>
real_t zVector<T>::Magnitude() const
{
    return sqrt((*this) * (*this));
}

template<typename U>
std::ostream& operator<<(std::ostream& out, const zVector<U>& V)
{
    out.setf(std::ios::fixed, std::ios::floatfield);
    out.precision(2);
    out << "<" << V.x << ", " << V.y << ", " << V.z << ">";
    return out;
}

template<typename T, typename U> static
real_t distance(const zVector<T>& A, const zVector<U>& B,
                const bool do_sqrt)
{
    // Just call the (x, y) version.
    return distance(A.x, A.y, B.x, B.y, do_sqrt);
}
