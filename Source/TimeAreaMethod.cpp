#include "../Headers/TimeAreaMethod.h"
#include "../Headers/FileWriter.h"

#include <iostream>
#include <iomanip>

void TimeAreaMethod::CalculateHydrographs(RainfallSeries rainfallseries, std::vector<Catchment> catchlist)
{
	std::cout << "Calculating Hydrographs\n";

	FileWriter::WriteTimeSeriestoCSV("./Exports/TimeSeries/Rainfall.csv", rainfallseries);

	
	for each (Catchment catchment in catchlist)
	{
		//Calculate Losses
		RainfallSeries rain;
		for (auto p : rainfallseries.series)
		{
			rain.series.push_back(p);
		}
		RainfallSeries InitialLosses;

		float ILremaining = catchment.IL;

		for (auto rainfall : rain.series)
		{
			Rainfall loss;
			loss.time = rainfall.time;
			if (rainfall.value >= ILremaining)
			{
				loss.value = ILremaining;
			}
			else
			{
				loss.value = rainfall.value;
				ILremaining -= rainfall.value;
			}
			InitialLosses.series.push_back(loss);
		}

		InitialLosses.ConvertToIntensity();
		rain.ConvertToIntensity();

		RainfallSeries ContinuingLosses;

		for (int i = 0; i < rain.series.size(); i++)
		{
			Rainfall loss;
			loss.time = rain.series[i].time;
			if (rain.series[i].value - InitialLosses.series[i].value >= catchment.CL)
			{
				loss.value = catchment.CL;
			}
			else
			{
				loss.value = rain.series[i].value;
			}

			ContinuingLosses.series.push_back(loss);
		}

		RainfallSeries CombinedLosses;

		for (int i = 0; i < rain.series.size(); i++)
		{
			Rainfall loss;
			loss.time = InitialLosses.series[i].time;
			loss.value = InitialLosses.series[i].value + ContinuingLosses.series[i].value;
			CombinedLosses.series.push_back(loss);
		}

		//Calculate Effective Rainfall Depth
		RainfallSeries EffIntensity;

		for (int i = 0; i < rain.series.size(); i++)
		{
			Rainfall eff;
			eff.time = InitialLosses.series[i].time;
			eff.value = rain.series[i].value - CombinedLosses.series[i].value;
			EffIntensity.series.push_back(eff);
		}

		FileWriter::WriteTimeSeriestoCSV("./Exports/TimeSeries/InitialLosses_" + std::to_string(catchment.id) + ".csv", InitialLosses);
		FileWriter::WriteTimeSeriestoCSV("./Exports/TimeSeries/ContinuingLosses_" + std::to_string(catchment.id) + ".csv", ContinuingLosses);
		FileWriter::WriteTimeSeriestoCSV("./Exports/TimeSeries/CombinedLosses_" + std::to_string(catchment.id) + ".csv", CombinedLosses);
		FileWriter::WriteTimeSeriestoCSV("./Exports/TimeSeries/EffectiveIntensity_" + std::to_string(catchment.id) + ".csv", EffIntensity);

		//Calculate Time Area Histogram and Hydrographs

		int timestep = EffIntensity.series[1].time - EffIntensity.series[0].time;
		int numtimesteps = catchment.isochroneareas.size() + EffIntensity.series.size();
		FlowSeries hydrographs;
		FlowSeries histographs;

		for (int i = 0; i < numtimesteps; i++)
		{
			FlowPoint flow;
			flow.time = i * timestep;
			flow.flow = 0;
			FlowPoint area;
			area.time = i * timestep;
			area.flow = 0;
			histographs.series.push_back(area);
			hydrographs.series.push_back(flow);
		}

		for (int i = 0; i < catchment.isochroneareas.size(); i++) //catchment index
		{
			for (int j = 1; j < EffIntensity.series.size();j++)
			{
				histographs.series[int(i+j)].flow += catchment.isochroneareas[i];
				hydrographs.series[int(i+j)].flow += ((float(catchment.isochroneareas[i]) / 10000) * EffIntensity.series[j].value)/360;
			}

		}

		FileWriter::WriteTimeSeriestoCSV("./Exports/TimeSeries/AreaHistograph_" + std::to_string(catchment.id) + ".csv", histographs);
		FileWriter::WriteTimeSeriestoCSV("./Exports/TimeSeries/Hydrograph_" + std::to_string(catchment.id) + ".csv", hydrographs);
	}
}