#include "../Headers/ReportManager.h"
#include <fstream>
#include <iomanip>

void ReportManager::CatchmentParameterReport(std::vector<Catchment> catchlist)
{
	std::ofstream outfile("./Exports/Reports/CatchmentReport.txt");

	outfile << "********************************************************************************************************************************\n";
	outfile << "*                                                Catchment Parameter Report                                                    *\n";
	outfile << "********************************************************************************************************************************\n";
	outfile << "\n";
	outfile << std::fixed << std::setprecision(4);

	for each (Catchment catchment in catchlist)
	{
		outfile << "Catchment ID: " << catchment.id << "\n";
		outfile << "Total Catchment Area: " << float(catchment.area) / 10000 << " ha\n";
		outfile << "Discharge Point Coordinates: " << catchment.dp.location.x << "," << catchment.dp.location.y << " (X,Y)\n";
		outfile << "Average manning's n value: " << std::defaultfloat << catchment.mannings << "\n";
		outfile << "Initial Loss: " << catchment.IL << " mm\n";
		outfile << "Continuing Loss: " << catchment.CL << " mm\n";
		outfile << "Longest Flow Path Length: " << catchment.longestfplength << " m\n";
		outfile << "Highest Point on Longest Flow Path: " << catchment.highestpt << " m\n";
		outfile << "Lowest Point on Longest Flow Path: " << catchment.lowestpt << " m\n";
		outfile << "Average Slope of Longest Flow Path: " << catchment.avgslope*100 << " %\n";
		outfile << "Flow Distance per Timestep: " << catchment.flowdistance << " m\n";

			//Begin Isochrone Area
		outfile << "Isochrone ID\t\t";
		for (int i = 0; i < catchment.isochroneareas.size(); i++)
		{
			outfile << i+1 << "\t";
		}

		outfile << "\nIsochrone Area (ha)\t";

		for each (int iso in catchment.isochroneareas)
		{
			float area = float(iso) / 10000;
			outfile << std::fixed << std::setprecision(4) << area << "\t";
		}

		outfile << "\n\n";
		outfile << "********************************************************************************************************************************\n";
		outfile << "\n";
	}
}
