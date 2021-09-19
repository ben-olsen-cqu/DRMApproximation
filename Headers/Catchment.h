#pragma once

#include "FlowClassification.h"
#include "MinMax.h"
#include <vector>

struct Catchment
{
	Catchment()
	{
	};
	int id;
	std::vector<Vec2> points;
	std::vector<int> isochroneareas;
	float mannings;
	float IL;
	float CL;
	float avgslope;
	float flowdistance;
	int area;
	FlowPath longest;
	float highestpt;
	float lowestpt;
	float longestfplength;
	DischargePoint dp;
	MinMax bounds;
};