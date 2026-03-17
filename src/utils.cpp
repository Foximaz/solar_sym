#include "utils.h"
#include <cmath>

Vector3::Vector3(double x, double y, double z) : x(x), y(y), z(z) {}

Vector3 Vector3::operator+(const Vector3& v) const { return Vector3(x + v.x, y + v.y, z + v.z); }
Vector3 Vector3::operator-(const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
Vector3 Vector3::operator*(double s) const { return Vector3(x * s, y * s, z * s); }

Vector3& Vector3::operator+=(const Vector3& v) { x += v.x; y += v.y; z += v.z; return *this; }
Vector3& Vector3::operator-=(const Vector3& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
Vector3& Vector3::operator*=(double s) { x *= s, y *= s, z *= s; return *this; }

double Vector3::length() const {
    return std::sqrt(x*x + y*y + z*z);
}

double Vector3::dot(const Vector3& v) const {
    return x * v.x + y * v.y + z * v.z;
}

Vector3 Vector3::normalized() const {
    double len = std::sqrt(x*x + y*y + z*z);
    if (len > 0) return Vector3(x/len, y/len, z/len);
    return *this;
}

Vector3 sphericalToCartesian(Vector3 centre, float yaw, float pitch, float distance) {
    float x = distance * cos(pitch) * sin(yaw);
    float y = distance * sin(pitch);
    float z = distance * cos(pitch) * cos(yaw);
    
    return centre + Vector3(x, y, z);
}

double lerp(double a, double b, double t) {
    return a + t * (b - a);
}
