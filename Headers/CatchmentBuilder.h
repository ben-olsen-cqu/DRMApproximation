#pragma once

#include "Quadtree.h"
#include "QuadtreeManager.h"
#include "Coordinates.h"
#include "CatchMath.h"
#include "FlowClassification.h"

#include <vector>

class CatchmentBuilder
{
public:
    void CreateCatchments(QuadtreeManager<Coordinates> &quad);
private:
    void SmoothPointsSingle(QuadtreeManager<Coordinates>& quad, QuadtreeManager<Coordinates>& smooth);
    void SmoothPointsSplit(QuadtreeManager<Coordinates>& quad, QuadtreeManager<Coordinates>& smooth);

    void CalculateNormalsSingle(QuadtreeManager<Coordinates>& smooth, QuadtreeManager<Normal>& normal);
    void CalculateNormalsSplit(QuadtreeManager<Coordinates>& smooth, QuadtreeManager<Normal>& normal);

    void CalculateFlowDirectionSingle(QuadtreeManager<FlowDirection>& flowdirection, QuadtreeManager<Normal>& normal);

    void CalculateFlowAccumulationSingle(QuadtreeManager<FlowDirection>& flowdirection, QuadtreeManager<FlowAccumulation>& flowaccum);

    std::vector<std::vector<Vec2>> StreamLinkingSingle(QuadtreeManager<FlowAccumulation>& flowaccum, QuadtreeManager<FlowDirection>& flowdirection);

    void TraceFlowPath(QuadtreeManager<FlowDirection>& flowdirection, std::vector<std::vector<Vec2>>* flowpaths, int x, int y);
};
