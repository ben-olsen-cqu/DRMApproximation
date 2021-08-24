#pragma once

#include <cmath>

enum class Direction { E = 0, NE = 1, N = 2, NW = 3, W = 4, SW = 5, S = 6, SE = 7 };

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

struct FlowAccumulation
{
public:
    double x;
    double y;
    double z;
    int flow;

    friend bool operator == (const FlowAccumulation& lhs, const FlowAccumulation& rhs)
    {
        if (std::fabs((lhs.x - rhs.x)) < 0.0001f)
            if (std::fabs((lhs.y - rhs.y)) < 0.0001f)
            {
                return true;
            }

        return false;

    }
    friend bool operator != (const FlowAccumulation lhs, const FlowAccumulation rhs)
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

    FlowAccumulation()
    {
        x = 0.0f;
        y = 0.0f;
        flow = 0;
    };

    FlowAccumulation(double _x, double _y)
    {
        x = _x;
        y = _y;
        flow = 0;
    };
    
    FlowAccumulation(double _x, double _y, double _z)
    {
        x = _x;
        y = _y;
        z = _z;
    };
    
    FlowAccumulation(double _x, double _y, int _flow)
    {
        x = _x;
        y = _y;
        flow = _flow;
    };
};
