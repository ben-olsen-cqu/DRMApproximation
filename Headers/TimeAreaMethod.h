#pragma once
#include "Catchment.h"
#include "HydrologyData.h"

#include <vector>

class TimeAreaMethod
{
public:
	static void CalculateHydrographs(RainfallSeries rainfallseries, std::vector<Catchment> catchlist);
};