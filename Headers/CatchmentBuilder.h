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
    void SmoothPoints(QuadtreeManager<Coordinates>& quad, QuadtreeManager<Coordinates>& smooth);
};
