#pragma once

#include <cmath>
#include <vector>

enum class Direction { 
    N = 0,
    NE = 1,
    E = 2,
    SE = 3,
    S = 4,
    SW = 5,
    W = 6,
    NW = 7
};

struct FlowDirection
{
public:
    double x;
    double y;
    double z;
    Direction direction;

    friend bool operator == (const FlowDirection& lhs, const FlowDirection& rhs)
    {
        if (std::fabs((lhs.x - rhs.x)) < 0.0001f)
            if (std::fabs((lhs.y - rhs.y)) < 0.0001f)
            {
                return true;
            }

        return false;

    }
    friend bool operator != (const FlowDirection lhs, const FlowDirection rhs)
    {
        if (lhs == rhs)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    FlowDirection()
    {
        x = 0.0f;
        y = 0.0f;
    };

    FlowDirection(double _x, double _y)
    {
        x = _x;
        y = _y;
    };

    FlowDirection(double _x, double _y, double _z)
    {
        x = _x;
        y = _y;
        z = _z;
    };


    FlowDirection(double _x, double _y, Direction d)
    {
        x = _x;
        y = _y;
        direction = d;
    };
};

struct FlowGeneral
{
public:
    double x;
    double y;
    double z;
    int iValue;

    friend bool operator == (const FlowGeneral& lhs, const FlowGeneral& rhs)
    {
        if (std::fabs((lhs.x - rhs.x)) < 0.0001f)
            if (std::fabs((lhs.y - rhs.y)) < 0.0001f)
            {
                return true;
            }

        return false;

    }
    friend bool operator != (const FlowGeneral lhs, const FlowGeneral rhs)
    {
        if (lhs == rhs)
        {
            return false;
        }
        else
        {
            return true;
        }
    }

    FlowGeneral()
    {
        x = 0.0f;
        y = 0.0f;
        iValue = 0;
    };

    FlowGeneral(double _x, double _y)
    {
        x = _x;
        y = _y;
        iValue = 0;
    };
    
    FlowGeneral(double _x, double _y, double _z)
    {
        x = _x;
        y = _y;
        z = _z;
    };
    
    FlowGeneral(double _x, double _y, int _flow)
    {
        x = _x;
        y = _y;
        iValue = _flow;
    };
};

struct FlowPath
{
    std::vector<Vec2> path;
    float Length()
    {
        float length = 0;
        for (int i = 0; i < path.size()-1; i++)
        {
            length += std::sqrt(std::pow(path[i + 1].x-path[i].x, 2) + std::pow(path[i + 1].y - path[i].y, 2));
        }
        return length;
    };
    Vec2 GetPointAtDist(float dist)
    {
        if (dist > this->Length())
        {
            return path[path.size() - 1];
        }
        if (dist <= 0)
        {
            return path[0];
        }

        float length = 0;
        
        for (int i = 0; i < path.size() - 1; i++)
        {
            length += std::sqrt(std::pow(path[i + 1].x - path[i].x, 2) + std::pow(path[i + 1].y - path[i].y, 2));
            if (length > dist)
            {
                return path[i];
            }
        }

    };
};

struct DischargePoint
{
    Vec2 loaction;
    int index;
};