#pragma once

#include <limits>

struct MinMax
{
	double minx = std::numeric_limits<double>::max();
	double miny = std::numeric_limits<double>::max();
	double minz = std::numeric_limits<double>::max();
	double maxx = std::numeric_limits<double>::min();
	double maxy = std::numeric_limits<double>::min();
	double maxz = std::numeric_limits<double>::min();
};