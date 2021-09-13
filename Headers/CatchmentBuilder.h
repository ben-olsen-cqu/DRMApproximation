#pragma once

#include "Quadtree.h"
#include "QuadtreeManager.h"
#include "Coordinates.h"
#include "CatchMath.h"
#include "FlowClassification.h"
#include "ProgramParameters.h"
#include "Catchment.h"

#include <vector>

class CatchmentBuilder
{
public:
    void CreateCatchments(ProgamParams progp);
private:
    //Pre-processing
    void SmoothPointsSingle(QuadtreeManager<Coordinates>& quad, QuadtreeManager<Coordinates>& smooth, int blurrad);
    void SmoothPointsSplit(QuadtreeManager<Coordinates>& quad, QuadtreeManager<Coordinates>& smooth, int blurrad);
    //Normals
    void CalculateNormalsSingle(QuadtreeManager<Coordinates>& smooth, QuadtreeManager<Normal>& normal);
    void CalculateNormalsSplit(QuadtreeManager<Coordinates>& smooth, QuadtreeManager<Normal>& normal);
    //Flow Directions
    void CalculateFlowDirectionSingle(QuadtreeManager<FlowDirection>& flowdirection, QuadtreeManager<Normal>& normal);
    void CalculateFlowDirectionSplit(QuadtreeManager<FlowDirection>& flowdirection, QuadtreeManager<Normal>& normal);
    //Flow Accum
    void CalculateFlowAccumulationSingle(QuadtreeManager<FlowDirection>& flowdirection, QuadtreeManager<FlowGeneral>& flowaccum);
    void CalculateFlowAccumulationSplit(QuadtreeManager<FlowDirection>& flowdirection, QuadtreeManager<FlowGeneral>& flowaccum);
    //Stream Linking
    std::vector<FlowPath> StreamLinkingSingle(QuadtreeManager<FlowGeneral>& flowaccum, QuadtreeManager<FlowDirection>& flowdirection, int acctarget);
    std::vector<FlowPath> StreamLinkingSplit(QuadtreeManager<FlowGeneral>& flowaccum, QuadtreeManager<FlowDirection>& flowdirection, int acctarget);
    void TraceFlowPath(QuadtreeManager<FlowDirection>& flowdirection, std::vector<std::vector<Vec2>>* flowpaths, int x, int y);
    //Catchment Classification
    void CatchmentClassification(QuadtreeManager<FlowGeneral>& catchclass, QuadtreeManager<FlowDirection>& flowdirection, std::vector<DischargePoint> dischargepoints);
    std::vector<DischargePoint> GenerateDischargePoints(std::vector<FlowPath>& fps, int breakdist);
    void ClassifyFlowPath(QuadtreeManager<FlowGeneral>& catchclass, QuadtreeManager<FlowDirection>& flowdirection, std::vector<DischargePoint> dischargepoints, Vec2 point);
    //Catchment Polygonisation
    void PolygoniseCatchments(QuadtreeManager<FlowGeneral>& catchclass, std::vector<DischargePoint> dischargepoints, std::vector<Catchment>& catchlist);
    float DistBetween(Vec2 v1, Vec2 v2);
};
