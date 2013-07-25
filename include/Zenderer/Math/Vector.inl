template<typename T>
template<typename U>
Vector<T>& Vector<T>::operator=(const Vector<U>& Copy)
{
    x = Copy.x;
    y = Copy.y;
    z = Copy.z;

    return *this;
}

template<typename T>
template<typename U>
bool Vector<T>::operator==(const Vector<U>& Other) const
{
    return compf(x, Other.x) && compf(y, Other.y) && compf(z, Other.z);
}

template<typename T>
template<typename U>
bool Vector<T>::operator!=(const Vector<U>& Other) const
{
    return !((*this) == Other);
}

template<typename T>
template<typename U>
Vector<T> Vector<T>::operator^(const Vector<U>& Other) const
{
    return Vector<T>(y * Other.z - z * Other.y,
                     x * Other.z - z * Other.x,
                     x * Other.y - y * Other.x);
}

template<typename T>
template<typename U>
real_t Vector<T>::operator*(const Vector<U>& Other) const
{
    return x * Other.x + y * Other.y + z * Other.z;
}

template<typename T>
Vector<T> Vector<T>::operator*(const real_t scalar) const
{
    return Vector<T>(x * scalar, y * scalar, z * scalar);
}

template<typename T>
Vector<T> Vector<T>::operator/(const real_t scalar) const
{
    return (*this) * (1 / scalar);
}

template<typename T>
template<typename U>
Vector<T> Vector<T>::operator+(const Vector<U>& Other) const
{
    return Vector<T>(x + Other.x, y + Other.y, z + Other.z);
}

template<typename T>
Vector<T> Vector<T>::operator+(const real_t value) const
{
    return Vector<T>(x + value, y + value, z + value);
}

template<typename T>
template<typename U>
Vector<T> Vector<T>::operator-(const Vector<U>& Other) const
{
    return Vector<T>(x - Other.x, y - Other.y, z - Other.z);
}

template<typename T>
void Vector<T>::Normalize()
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
void Vector<T>::Rotate(const real_t radians)
{
    real_t c = cos(radians);
    real_t s = sin(radians);
    real_t old_x = x;

    x = x * c - y * s;
    y = old_x * s + y * c;
}

template<typename T>
template<typename U>
real_t Vector<T>::Cross2D(const Vector<U>& Other) const
{
    return ((*this) ^ Other).Magnitude();
}

template<typename T>
Vector<T> Vector<T>::GetNormalized() const
{
    real_t mag = this->Magnitude();
    return Vector<T>(x / mag, y / mag);
}

template<typename T>
real_t Vector<T>::Magnitude() const
{
    return sqrt((*this) * (*this));
}

template<typename U>
std::ostream& operator<<(std::ostream& out, const Vector<U>& V)
{
    out.setf(std::ios::fixed, std::ios::floatfield);
    out.precision(2);
    out << "<" << V.x << ", " << V.y << ", " << V.z << ">";
    return out;
}

template<typename T, typename U> static
real_t distance(const Vector<T>& A, const Vector<U>& B,
                const bool do_sqrt)
{
    real_t pyth = (A.x + B.x) * (A.x + B.x) +
                  (A.y + B.y) * (A.y + B.y);
    return !do_sqrt ? pyth : std::sqrt(pyth);
}
