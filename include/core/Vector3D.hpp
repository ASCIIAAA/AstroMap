#ifndef VECTOR3D_HPP
#define VECTOR3D_HPP

#include <cmath>

class Vector3D {
public:
    double x, y, z;

    Vector3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}

    // Inlined operations for maximum speed
    inline Vector3D operator+(const Vector3D& v) const { return {x + v.x, y + v.y, z + v.z}; }
    inline Vector3D operator-(const Vector3D& v) const { return {x - v.x, y - v.y, z - v.z}; }
    inline Vector3D operator*(double s) const { return {x * s, y * s, z * s}; }

    inline double dot(const Vector3D& v) const { return x * v.x + y * v.y + z * v.z; }
    
    inline double magnitudeSquared() const { return x * x + y * y + z * z; }
    inline double magnitude() const { return std::sqrt(magnitudeSquared()); }

    inline double distanceTo(const Vector3D& v) const {
        return (*this - v).magnitude();
    }
};

#endif