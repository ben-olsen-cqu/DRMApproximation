#pragma once

#include "Catchment.h"
#include <vector>

class ReportManager
{
public:
	static void CatchmentParameterReport(std::vector<Catchment> catchlist);
};