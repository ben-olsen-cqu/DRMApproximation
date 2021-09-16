#pragma once
#include <vector>

struct Rainfall
{
	int time; //should always be in mins
	float value; //can be intesity or depth
	Rainfall(int _time, float _value)
	{
		time = _time;
		value = _value;
	}

	Rainfall()
	{
		time = 0;
		value = 0;
	}
};

struct RainfallSeries
{
	std::vector<Rainfall> series;
public:
	void ConvertToDepth()
	{
		if (series.size() > 1)
		{
			float timestep = series[1].time - series[0].time;

			for(int i = 0; i < series.size(); i++)
			{
				series[i].value = series[i].value * (timestep/60);
			}

		}
	}
	void ConvertToIntensity()
	{
		if (series.size() > 1)
		{
			float timestep = series[1].time - series[0].time;

			for (int i = 0; i < series.size(); i++)
			{
				series[i].value = series[i].value * (60 / timestep);
			}

		}
	}
};

struct FlowPoint
{
	int time; //should always be in mins
	float flow; //is always in cumecs
};

struct FlowSeries
{
	std::vector<FlowPoint> series;
};