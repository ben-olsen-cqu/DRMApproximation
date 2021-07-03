#pragma once

#include <math.h>

class Vec2
{
public:
    double x;
    double y;
public:
    Vec2()
    {
        x = 0;
        y = 0;
    }
    Vec2(double _x, double _y)
    {
        x= _x;
        y = _y;
    }
    
    Vec2& operator = (const Vec2 & rhs)
    {
        x = rhs.x;
        y = rhs.y;
        return *this;
    }
    
    Vec2 operator | (const Vec2 & rhs)
    {
        return Vec2(-x,-y);
    }
    
    Vec2& operator += (const Vec2 &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
    
    Vec2& operator -= (const Vec2 &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }
    
    Vec2& operator /= ( const double &rhs )
    {
        x /= rhs;
        y /= rhs;
        return *this;
    }
    Vec2 operator / ( const double &rhs ) const
    {
        return Vec2( *this ) /= rhs;
    }
    
    Vec2 operator + ( const Vec2 &rhs ) const
    {
        return Vec2( *this ) += rhs;
    }
    
    Vec2 operator - ( const Vec2 &rhs ) const
    {
        return Vec2( *this ) -= rhs;
    }
    
    double Length()
    {
        return std::sqrt(((x*x) + (y*y)));
    }
    
    Vec2& Normalize()
    {
        const double length = Length();
        x /= length;
        y /= length;
        return *this;
    }
    Vec2 GetNormalized() const
    {
        Vec2 norm = *this;
        norm.Normalize();
        return norm;
    }
    Vec2& operator*=( const double &rhs )
    {
        x *= rhs;
        y *= rhs;
        return *this;
    }
    Vec2 operator*( const double &rhs ) const
    {
        return Vec2( *this ) *= rhs;
    }
    
    double operator*( const Vec2 &rhs ) const
    {
        return x * rhs.x + y * rhs.y;
    }
    
    bool    operator==( const Vec2 &rhs ) const
    {
        if (std::fabs((x - rhs.x)) < 0.0001f)
            if (std::fabs((y - rhs.y)) < 0.0001f)
            {
                return true;
            }

        return false;

    }
    bool    operator!=( const Vec2 &rhs ) const
    {
        return !(*this == rhs);
    }
    
    Vec2& Rotate() //clockwise rotation of 90 degrees
    {
        double temp = -x;
        x = y;
        y = temp;
        return *this;
    }
    
};

class Vec3 : public Vec2
{
public:
    double z;
public:
    Vec3() = default;
    Vec3(double _x, double _y)
    :
    Vec2(_x,_y),
    z(0)
    {}
    Vec3(double _x, double _y, double _z)
    :
    Vec2(_x,_y),
    z(_z)
    {}
    
    Vec3& operator = (const Vec3 & rhs)
    {
        x = rhs.x;
        y = rhs.y;
        z = rhs.z;
        return *this;
    }
    
    Vec3 operator | (const Vec3 & rhs)
    {
        return Vec3(-x,-y,-z);
    }
    
    Vec3& operator += (const Vec3 &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }
    
    Vec3& operator -= (const Vec3 &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }
    
    Vec3& operator /= ( const double &rhs )
    {
        x /= rhs;
        y /= rhs;
        z /= rhs;
        return *this;
    }
    Vec3 operator / ( const double &rhs ) const
    {
        return Vec3( *this ) /= rhs;
    }
    
    Vec3 operator + ( const Vec3 &rhs ) const
    {
        return Vec3( *this ) += rhs;
    }
    
    Vec3 operator - ( const Vec3 &rhs ) const
    {
        return Vec3( *this ) -= rhs;
    }
    
    double Length()
    {
        return std::sqrt(((x*x) + (y*y) + (z*z)));
    }
    
    Vec3& Normalize()
    {
        const double length = Length();
        x /= length;
        y /= length;
        z /= length;
        return *this;
    }

    Vec3& Inverse()
    {
        const double length = Length();
        x *= -1;
        y *= -1;
        z *= -1;
        return *this;
    }

    Vec3 GetNormalized() const
    {
        Vec3 norm = *this;
        norm.Normalize();
        return norm;
    }
    Vec3& operator*=( const double &rhs )
    {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }
    Vec3 operator*( const double &rhs ) const
    {
        return Vec3( *this ) *= rhs;
    }
    
    double operator*( const Vec3 &rhs ) const
    {
        return x * rhs.x + y * rhs.y + z * rhs.z;
    }
    
    bool    operator==( const Vec3 &rhs ) const
    {
        if (std::fabs((x - rhs.x)) < 0.0001f)
            if (std::fabs((y - rhs.y)) < 0.0001f)
                if (std::fabs((z - rhs.z)) < 0.0001f)
                {
                    return true;
                }
        
        return false;
    }
    bool    operator!=( const Vec3 &rhs ) const
    {
        return !(*this == rhs);
    }
    
    Vec3 operator % (const Vec3 rhs) //Cross Product
    {
        return Vec3((y*rhs.z - z*rhs.y),(z*rhs.x-x*rhs.z),(x*rhs.y-y*rhs.x) );
    }
    
};
class Normal : public Vec3
{
public:
    Vec3 norm;
    Normal() = default;
    Normal(double x, double y)
    :
    Vec3(x,y,0),
    norm(0,0,0)
    {}
    Normal(double x, double y, double z)
    :
    Vec3(x,y,z),
    norm(0,0,0)
    {}
    Normal(Vec3 translation, Vec3 normal)
    :
    Vec3(translation),
    norm(normal)
    {}
    bool    operator==( const Normal &rhs ) const
    {
        if (std::fabs((x - rhs.x)) < 0.0001f)
            if (std::fabs((y - rhs.y)) < 0.0001f)
                if (std::fabs((z - rhs.z)) < 0.0001f)
                {
                    return true;
                }
        
        return false;
    }
};
