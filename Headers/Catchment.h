#pragma once

#include "CatchMath.h"
#include "FlowClassification.h"

#include <vector>

struct Catchment
{
	Catchment()
	{
	};
	int id;
	std::vector<Vec2> points;
	std::vector<float> isochroneareas;
	float mannings;
	float IL;
	float CL;
	float avgslope;
	float flowdistance;
	float area;
	FlowPath longest;
	float highestpt;
	float lowestpt;
	float longestfplength;
	DischargePoint dp;
};