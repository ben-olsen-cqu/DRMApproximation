#pragma once
#include <cmath>
struct Coordinates
{
public:
    double x;
    double y;
    double z;
    
    friend bool operator == (const Coordinates &lhs, const Coordinates &rhs)
    {
        if (std::fabs((lhs.x - rhs.x)) < 0.0001f)
            if (std::fabs((lhs.y - rhs.y)) < 0.0001f)
            {
                return true;
            }

        return false;

    }
    friend bool operator != (const Coordinates lhs, const Coordinates rhs)
    {
        if(lhs == rhs)
        {
            return false;
        }
        else
        {
            return true;
        }
    }
    
    Coordinates()
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
    };
    
    Coordinates(double _x, double _y)
    {
        x = _x;
        y = _y;
        z = 0.0f;
    };
    
    Coordinates(double _x, double _y, double _z)
    {
        x = _x;
        y = _y;
        z = _z;
    };
};
