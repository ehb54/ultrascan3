// sesmesh: a small 3-vector type.
#pragma once

#include <cmath>

namespace sesmesh {

struct Vec3 {
   double x = 0, y = 0, z = 0;

   Vec3() = default;
   Vec3( double a, double b, double c ) : x( a ), y( b ), z( c ) {}

   double operator[]( int i ) const { return i == 0 ? x : ( i == 1 ? y : z ); }
   double& operator[]( int i ) { return i == 0 ? x : ( i == 1 ? y : z ); }

   Vec3 operator+( const Vec3& o ) const { return { x + o.x, y + o.y, z + o.z }; }
   Vec3 operator-( const Vec3& o ) const { return { x - o.x, y - o.y, z - o.z }; }
   Vec3 operator-() const { return { -x, -y, -z }; }
   Vec3 operator*( double s ) const { return { x * s, y * s, z * s }; }
   Vec3 operator/( double s ) const { return { x / s, y / s, z / s }; }
   Vec3& operator+=( const Vec3& o ) {
      x += o.x;
      y += o.y;
      z += o.z;
      return *this;
   }
   Vec3& operator-=( const Vec3& o ) {
      x -= o.x;
      y -= o.y;
      z -= o.z;
      return *this;
   }
   Vec3& operator*=( double s ) {
      x *= s;
      y *= s;
      z *= s;
      return *this;
   }
};

inline Vec3 operator*( double s, const Vec3& v ) { return v * s; }
inline double dot( const Vec3& a, const Vec3& b ) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline Vec3 cross( const Vec3& a, const Vec3& b ) {
   return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}
inline double norm2( const Vec3& a ) { return dot( a, a ); }
inline double norm( const Vec3& a ) { return std::sqrt( dot( a, a ) ); }
inline Vec3 unit( const Vec3& a ) {
   double n = norm( a );
   return n > 0 ? a / n : Vec3();
}

// Unit vectors u, v such that (u, v, n) is a right-handed orthonormal basis; n must be a unit vector.
inline void basis( const Vec3& n, Vec3& u, Vec3& v ) {
   Vec3 a = std::fabs( n.x ) < 0.6 ? Vec3( 1, 0, 0 ) : ( std::fabs( n.y ) < 0.6 ? Vec3( 0, 1, 0 ) : Vec3( 0, 0, 1 ) );
   u = unit( a - n * dot( a, n ) );
   v = cross( n, u );
}

}  // namespace sesmesh
