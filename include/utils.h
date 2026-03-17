#ifndef UTILS_H
#define UTILS_H

#include <cmath>

struct Vector3 {
    double x, y, z;
    
    Vector3(double x = 0, double y = 0, double z = 0);
    
    Vector3 operator+(const Vector3& v) const;
    Vector3 operator-(const Vector3& v) const;
    Vector3 operator*(double s) const;
    
    Vector3& operator+=(const Vector3& v);
    Vector3& operator-=(const Vector3& v);
    Vector3& operator*=(double s);
    
    double length() const;
    double dot(const Vector3& v) const;
    Vector3 normalized() const;
};

Vector3 sphericalToCartesian(Vector3 centre, float yaw, float pitch, float distance);

double lerp(double a, double b, double t);

#endif
