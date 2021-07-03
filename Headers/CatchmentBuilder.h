#pragma once

#include "Quadtree.h"
#include "Coordinates.h"
#include "CatchMath.h"

#include <vector>

class CatchmentBuilder
{
public:
    void CreateCatchments(const Quadtree<Coordinates>* quad);
    Quadtree<Coordinates>* SmoothPoints(const Quadtree<Coordinates>* quad);
};
