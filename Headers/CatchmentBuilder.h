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
    void SmoothPointsSingle(QuadtreeManager<Coordinates>& quad, QuadtreeManager<Coordinates>& smooth);
    void SmoothPointsSplit(QuadtreeManager<Coordinates>& quad, QuadtreeManager<Coordinates>& smooth);
    void CalculateNormals(QuadtreeManager<Coordinates>& quad, QuadtreeManager<Normal>& normal);
};
