#pragma once

#include "Quadtree.h"
#include "QuadtreeManager.h"
#include "Coordinates.h"
#include "CatchMath.h"

#include <vector>

class CatchmentBuilder
{
public:
    void CreateCatchments(QuadtreeManager<Coordinates> &quad);
private:
    Quadtree<Coordinates>* SmoothPoints(const Quadtree<Coordinates>* quad);
};
