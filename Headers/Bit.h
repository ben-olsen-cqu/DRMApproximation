#pragma once

struct Bit
{
public:
    double x;
    double y;
    bool value; // false for below, true for above

    friend bool operator == (const Bit lhs, const Bit rhs)
    {
        if ((lhs.x - rhs.x) < 0.0001f)
            if ((lhs.y - rhs.y) < 0.0001f)
            {
                return true;
            }

        return false;

    }

    Bit()
    {
        x = 0.0f;
        y = 0.0f;
        value = false;
    };
    Bit(double _x, double _y)
    {
        x = _x;
        y = _y;
        value = false;
    };

    Bit(double _x, double _y, bool _value)
    {
        x = _x;
        y = _y;
        value = _value;
    };

};