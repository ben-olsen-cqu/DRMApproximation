#include "../Headers/FileWriter.h"
#include <fstream>
#include <iostream>

void FileWriter::WriteFiles(std::string filename, std::vector<std::string> data, bool multiplefiles)
{
	if (multiplefiles)
	{
		for (int i = 0; i < data.size(); i++)
			Write(filename + std::to_string(i) + ".csv", data[i]);
	}
	else
	{
		Write(filename + ".csv", data);
	}
}

void FileWriter::WriteCoordTree(std::string filename, QuadtreeManager<Coordinates>& quad)
{
    std::ofstream outfile(filename + ".csv");

	float boundsx = (quad.BottomRight().x) - (quad.TopLeft().x);
	float boundsy = (quad.TopLeft().y) - (quad.BottomRight().y);
	float bottom = (quad.BottomRight().y);
	float left = (quad.TopLeft().x);

	for (int x = 0; x < boundsx; x++)
		for (int y = 0; y < boundsy; y++)
		{
			Coordinates c = quad.Search(Coordinates(x + left, y + bottom))->pos;
			std::string line = std::to_string(c.x) + "," + std::to_string(c.y) + "," + std::to_string(c.z) + ",";
            outfile << line << std::endl;
		}
	outfile.close();
}

void FileWriter::WriteVec2Points(std::string filename, std::vector<DischargePoint> points)
{
	std::ofstream outfile(filename + ".csv");
	outfile << "POINT ()\n";

	if (points.size() > 0)
	{
		for each (DischargePoint dp in points)
		{
			std::string line = "POINT (" + std::to_string(dp.location.x) + " " + std::to_string(dp.location.y) + ")";
			outfile << line << std::endl;
		}
	}

	outfile.close();
}

void FileWriter::WriteCoordTreeASC(std::string filename, QuadtreeManager<Coordinates>& quad)
{
	if (quad.type == TreeType::Single)
	{
		std::ofstream outfile(filename + ".asc");

		float boundsx = (quad.BottomRight().x) - (quad.TopLeft().x);
		float boundsy = (quad.TopLeft().y) - (quad.BottomRight().y);
		float bottom = (quad.BottomRight().y);
		float left = (quad.TopLeft().x);
		int ncols = int(boundsx);
		int nrows = int(boundsy);

		outfile << std::fixed;
		outfile << "ncols\t\t\t" << (ncols) << "\n";
		outfile << "nrows\t\t\t" << (nrows) << "\n";
		outfile << "xllcorner\t\t" << left << "\n";
		outfile << "yllcorner\t\t" << bottom << "\n";
		outfile << "cellsize\t\t" << 1.0f << "\n";
		outfile << "NODATA_value\t" << -9999 << "\n ";

		for (int y = 0; y < boundsy; y++)
		{
			for (int x = 0; x < boundsx; x++)
			{
				auto row = (boundsy - y) + bottom;
				auto column = x + left;
				auto n = quad.Search(Coordinates(column, row));

				if (n != nullptr)
				{
					Coordinates c = n->pos;
					std::string line = std::to_string(c.z) + " ";
					outfile << line;
				}
				else
				{
					outfile << "-9999";
				}

			}
			outfile << std::endl << " ";
		}
		outfile.close();
	}
	else
	{
		double boundsx = (quad.BottomRight().x) - (quad.TopLeft().x);
		double boundsy = (quad.TopLeft().y) - (quad.BottomRight().y);
		double bottom = (quad.BottomRight().y);
		double left = (quad.TopLeft().x);

		int numquads = std::pow(2, quad.splitlevel); //quad splits the area in half in the x and y axis
		int totalquads = numquads * numquads; //total number of sub quads at the split level

		double boundspersubquadx = boundsx / numquads; //the equal division of space in the x axis for the split level
		double boundspersubquady = boundsy / numquads; //the equal division of space in the y axis for the split level

		//for loops for iterating through split level trees
		for (int v = 0; v < numquads; v++) //move vertically through sub trees
		{
			//Calculate the offset to add when seaching within the loop, the +1 is to push it into the next subquad if subquad is the bottom left no offset is required
			//int offsety = v == 0 ? 0 : std::floor(boundspersubquady * v)+1;

			int offsety;
			if (v == 0)
			{
				offsety = 0; //confirmed
				boundspersubquady = 882; //confirmed
			}
			if (v == 1)
			{
				offsety = 882; //confirmed
				boundspersubquady = 882; //confirmed
			}
			if (v == 2)
			{
				offsety = 1764; //confirmed
				boundspersubquady = 881; //confirmed
			}
			if (v == 3)
			{
				offsety = 2645; //confirmed
				boundspersubquady = 882; //confirmed
			}
			if (v == 4)
			{
				offsety = 3527; //confirmed
				boundspersubquady = 881; //confirmed
			}
			if (v == 5)
			{
				offsety = 4408; //confirmed
				boundspersubquady = 882; //confirmed
			}
			if (v == 6)
			{
				offsety = 5290;
				boundspersubquady = 881;
			}
			if (v == 7)
			{
				offsety = 6171;
				boundspersubquady = 882;
			}

			for (int w = 0; w < numquads; w++) //move horizontally through sub trees
			{
				//Same as the y offset
				//int offsetx = w == 0 ? 0 : std::floor(boundspersubquadx * w)+1;

				int offsetx;
				if (w == 0) //Quad 1
				{
					offsetx = 0;
					boundspersubquadx = 882;
				}
				if (w == 1) //Quad 2
				{
					offsetx = 882;
					boundspersubquadx = 881;
				}
				if (w == 2) //Quad 3
				{
					offsetx = 1763;
					boundspersubquadx = 882;
				}
				if (w == 3) //Quad 4
				{
					offsetx = 2645;
					boundspersubquadx = 881;
				}
				if (w == 4) //Quad 5
				{
					offsetx = 3526;
					boundspersubquadx = 882;
				}
				if (w == 5) //Quad 6
				{
					offsetx = 4408;
					boundspersubquadx = 881;
				}
				if (w == 6) //Quad 7
				{
					offsetx = 5289;
					boundspersubquadx = 882;
				}
				if (w == 7) //Quad 8
				{
					offsetx = 6171;
					boundspersubquadx = 882;
				}
				std::cout << "\rProcessing Quad " << v * numquads + (w + 1) << " of " << totalquads;

				std::ofstream outfile(filename + "_" + std::to_string(v * numquads + (w + 1)) + ".asc");
				outfile << std::fixed;
				outfile << "ncols\t\t\t" << (boundspersubquady) << "\n";
				outfile << "nrows\t\t\t" << (boundspersubquadx) << "\n";
				outfile << "xllcorner\t\t" << (offsetx + left) << "\n";
				outfile << "yllcorner\t\t" << (offsety + bottom) << "\n";
				outfile << "cellsize\t\t" << quad.spacing << "\n";
				outfile << "NODATA_value\t" << -9999 << "\n ";

				for (int y = 0; y < boundspersubquady; y++) //move through each coord in the y direction of the subtree
				{
					for (int x = 0; x < boundspersubquadx; x++)//move through each coord in the x direction of the subtree
					{
						auto row = (offsety - y) + bottom;
						auto column = x + + offsetx + left;

						auto n = quad.Search(Coordinates(column, row));

						if (n != nullptr)
						{
							Coordinates c = n->pos;
							std::string line = std::to_string(c.z) + " ";
							outfile << line;
						}
						else
						{
							outfile << "-9999 ";
						}
					}
					outfile << std::endl << " ";
				}
				outfile.close();
			}
		}
	}
}

void FileWriter::WriteFlowGeneralTreeASC(std::string filename, QuadtreeManager<FlowGeneral>& quad)
{
	if (quad.type == TreeType::Single)
	{
		std::ofstream outfile(filename + ".asc");

		float boundsx = (quad.BottomRight().x) - (quad.TopLeft().x);
		float boundsy = (quad.TopLeft().y) - (quad.BottomRight().y);
		float bottom = (quad.BottomRight().y);
		float left = (quad.TopLeft().x);
		int ncols = int(boundsx);
		int nrows = int(boundsy);

		outfile << std::fixed;
		outfile << "ncols\t\t\t" << (ncols) << "\n";
		outfile << "nrows\t\t\t" << (nrows) << "\n";
		outfile << "xllcorner\t\t" << left << "\n";
		outfile << "yllcorner\t\t" << bottom << "\n";
		outfile << "cellsize\t\t" << 1.0f << "\n";
		outfile << "NODATA_value\t" << -9999 << "\n ";

		for (int y = 0; y < boundsy; y++)
		{
			for (int x = 0; x < boundsx; x++)
			{
				auto row = (boundsy - y) + bottom;
				auto column = x + left;
				if ((quad.Search(FlowGeneral(column, row))) != nullptr)
				{
					FlowGeneral c = quad.Search(FlowGeneral(column, row))->pos;
					std::string line = std::to_string(c.iValue) + " ";
					outfile << line;
				}
				else
				{
					outfile << "-9999 ";
				}
			}
			outfile << std::endl << " ";
		}
		outfile.close();
	}
	//else
	//{
		//for (int i = 0; quad.bottomnodes.size(); i++)
		//{
		//	std::ofstream outfile(filename + std::to_string(i) + ".asc");

		//	double boundsx = (quad.bottomnodes[i]->BottomRight().x) - (quad.bottomnodes[i]->TopLeft().x);
		//	double boundsy = (quad.bottomnodes[i]->TopLeft().y) - (quad.bottomnodes[i]->BottomRight().y);
		//	double bottom = (quad.bottomnodes[i]->BottomRight().y);
		//	double left = (quad.bottomnodes[i]->TopLeft().x);
		//	int ncols = int(boundsx);
		//	int nrows = int(boundsy);

		//	outfile << std::fixed;
		//	outfile << "ncols\t\t\t" << (ncols) << "\n";
		//	outfile << "nrows\t\t\t" << (nrows) << "\n";
		//	outfile << "xllcorner\t\t" << left << "\n";
		//	outfile << "yllcorner\t\t" << bottom << "\n";
		//	outfile << "cellsize\t\t" << quad.spacing << "\n";
		//	outfile << "NODATA_value\t" << -9999 << "\n ";
//
//			for (int y = 0; y < boundsy; y++)
//			{
//				for (int x = 0; x < boundsx; x++)
//				{
//					auto row = (boundsy - y) + bottom;
//					auto column = x + left;
//
//					auto n = quad.Search(FlowAccumulation(column, row));
//
//					if (n != nullptr)
//					{
//						FlowAccumulation c = n->pos;
//						std::string line = std::to_string(c.flow) + " ";
//						outfile << line;
//					}
//				}
//				outfile << std::endl << " ";
//			}
	//	}
	//}
}

void FileWriter::Write(std::string filename, std::string data)
{
	std::ofstream outfile(filename);

	outfile << data;

	outfile.close();

}

void FileWriter::Write(std::string filename, std::vector<std::string> data)
{
	std::ofstream outfile(filename);

	for (auto s : data)
		outfile << s + "\n";

	outfile.close();

}

void FileWriter::WriteVecNormals3dWKT(std::string filename, QuadtreeManager<Normal>& normals) {
	if (normals.type == TreeType::Single)
	{
		std::ofstream outfile(filename + ".csv");
		outfile << "LINESTRING ()\n";
		float boundsx = (normals.BottomRight().x) - (normals.TopLeft().x);
		float boundsy = (normals.TopLeft().y) - (normals.BottomRight().y);
		float bottom = (normals.BottomRight().y);
		float left = (normals.TopLeft().x);

		for (int x = 0; x <= boundsx; x++)
			for (int y = 0; y <= boundsy; y++)
			{
				auto c = normals.Search(Normal(x + left, y + bottom))->pos;
				std::string line = "LINESTRING (" + std::to_string(c.x) + " " + std::to_string(c.y) + " " + std::to_string(c.z) + ","
					+ std::to_string(c.norm.x) + " " + std::to_string(c.norm.y) + " " + std::to_string(c.norm.z) + ")";
				outfile << line << std::endl;
			}
		outfile.close();
	}
	else
	{

	}
}

void FileWriter::WriteFlowDirection2dWKT(std::string filename, QuadtreeManager<FlowDirection>& normals)
{
	if (normals.type == TreeType::Single)
	{
		std::ofstream outfile(filename + ".csv");

		outfile << "LINESTRING ()\n";

		float boundsx = (normals.BottomRight().x) - (normals.TopLeft().x);
		float boundsy = (normals.TopLeft().y) - (normals.BottomRight().y);
		float bottom = (normals.BottomRight().y);
		float left = (normals.TopLeft().x);

		for (int x = 0; x <= boundsx; x++)
			for (int y = 0; y <= boundsy; y++)
			{
				auto node = normals.Search(FlowDirection(x + left, y + bottom));
				
				if (node != nullptr)
				{
					auto d = node->pos;

					Vec2 i;
					Vec2 j;

					i.x = d.x;
					i.y = d.y;
					j.x = d.x;
					j.y = d.y;

					if (d.direction == Direction::NE)
					{
						j.x += 0.5;
						j.y += 0.5;
					}
					if (d.direction == Direction::N)
					{
						j.y += 0.5;
					}
					if (d.direction == Direction::NW)
					{
						j.x -= 0.5;
						j.y += 0.5;
					}
					if (d.direction == Direction::E)
					{
						j.x += 0.5;
					}
					if (d.direction == Direction::W)
					{
						j.x -= 0.5;
					}
					if (d.direction == Direction::SE)
					{
						j.x += 0.5;
						j.y -= 0.5;
					}
					if (d.direction == Direction::S)
					{
						j.y -= 0.5;
					}
					if (d.direction == Direction::SW)
					{
						j.x -= 0.5;
						j.y -= 0.5;
					}

					j = j - i;
					j = j + i;

					std::string line = "LINESTRING (" + std::to_string(i.x) + " " + std::to_string(i.y) + "," + std::to_string(j.x) + " " + std::to_string(j.y) + ")";
					outfile << line << std::endl;
				}
			}
		outfile.close();
	}
	else
	{

	}
}

void FileWriter::WriteVecNormals2dWKT(std::string filename, QuadtreeManager<Normal>& normals) {
	
	if (normals.type == TreeType::Single)
	{
		std::ofstream outfile(filename + ".csv");
		outfile << "LINESTRING ()\n";

		float boundsx = (normals.BottomRight().x) - (normals.TopLeft().x);
		float boundsy = (normals.TopLeft().y) - (normals.BottomRight().y);
		float bottom = (normals.BottomRight().y);
		float left = (normals.TopLeft().x);

		for (int x = 0; x <= boundsx; x++)
			for (int y = 0; y <= boundsy; y++)
			{
				auto n = normals.Search(Normal(x + left, y + bottom));
				if (n != nullptr)
				{
					auto c = n->pos;
					std::string line = "LINESTRING (" + std::to_string(c.x) + " " + std::to_string(c.y) + "," + std::to_string(c.norm.x) + " " + std::to_string(c.norm.y) + ")";
					outfile << line << std::endl;
				}
			}
		outfile.close();
	}
	else
	{
		float boundsx = (normals.BottomRight().x) - (normals.TopLeft().x);
		float boundsy = (normals.TopLeft().y) - (normals.BottomRight().y);
		float bottom = (normals.BottomRight().y);
		float left = (normals.TopLeft().x);

		int numquads = normals.splitlevel * 2; //quad splits the area in half in the x and y axis

		int boundsperquadx = std::floor(boundsx / numquads);
		int boundsperquady = std::floor(boundsy / numquads);

		int totalquads = numquads * numquads;

		for (int v = 0; v < numquads; v++) //move vertically through sub trees
			for (int w = 0; w < numquads; w++) //move horizontally through sub trees
			{
				std::cout << "\rProcessing Quad " << v * numquads + (w + 1) << " of " << totalquads;
				std::ofstream outfile(filename + "_" + std::to_string(w + v*numquads) +".csv");
				outfile << "LINESTRING ()\n";
				for (int y = 0; y < boundsperquady; y++) //move through each coord in the y direction of the subtree
					for (int x = 0; x < boundsperquadx; x++)//move through each coord in the x direction of the subtree
					{
						auto c = normals.Search(Normal(x + w * boundsperquadx + left, y + v * boundsperquady + bottom));
						if (c != nullptr)
						{
							std::string line = "LINESTRING (" + std::to_string(c->pos.x) + " " + std::to_string(c->pos.y) + "," + std::to_string(c->pos.norm.x) + " " + std::to_string(c->pos.norm.y) + ")";
							outfile << line << std::endl;
						}
					}

				outfile.close();
			}
		std::cout << "\n";
	}
}

void FileWriter::WriteStreamPaths2dWKT(std::string filename, std::vector<Catchment>& catchlist)
{
	std::ofstream outfile(filename + ".csv");
	outfile << "LINESTRING ()\n";

	for each (Catchment list in catchlist)
	{
		std::string line = "LINESTRING (";

		for each (Vec2 point in list.longest.path)
		{
			line += std::to_string(point.x) + " " + std::to_string(point.y) + ",";
		}
		line.erase(line.size() - 1, 1);
		line += ")";
		outfile << line << std::endl;
	}
	outfile.close();
}

void FileWriter::WriteStreamPaths2dWKT(std::string filename, std::vector<FlowPath>& flowpaths)
{
	std::ofstream outfile(filename + ".csv");
	outfile << "LINESTRING ()\n";

	for each (FlowPath list in flowpaths)
	{
		std::string line = "LINESTRING (";

		for each (Vec2 point in list.path)
		{
			line += std::to_string(point.x) + " " + std::to_string(point.y) + ",";
		}
		line.erase(line.size() - 1, 1);
		line += ")";
		outfile << line << std::endl;
	}
	outfile.close();
}

void FileWriter::WriteCatchmentPolysWKT(std::string filename, std::vector<Catchment>& catchlist)
{
	std::ofstream outfile(filename + ".csv");
	outfile << "POLYGON (( ))\n";

	for each (Catchment cat in catchlist)
	{
		std::string line = "POLYGON ((";

		for each (Vec2 point in cat.points)
		{
			line += std::to_string(point.x) + " " + std::to_string(point.y) + ",";
		}
		line += std::to_string(cat.points[0].x) + " " + std::to_string(cat.points[0].y);
		line += "))";
		outfile << line << std::endl;
	}
	outfile.close();
}

void FileWriter::WriteStreamPathsBinary(std::string filepath, std::vector<FlowPath>& flowpaths)
{
	std::ofstream datastream;

	datastream.open("./" + filepath + ".bin", std::ios::binary);

	for each (FlowPath var in flowpaths)
	{
		int size = var.path.size();
		datastream.write((char*)&var.id, sizeof(int)); 
		datastream.write((char*)&size, sizeof(int));

		for (int i = 0; i < size; i++)
		{
			datastream.write((char*)&var.path[i], sizeof(Vec2));
		}
	}

	datastream.close();
}

void FileWriter::WriteCatchmentstoBinary(std::string filepath, std::vector<Catchment>& catchments)
{
	std::ofstream datastream;

	datastream.open("./" + filepath + ".bin", std::ios::binary);

	for each (Catchment var in catchments)
	{

		datastream.write((char*)&var.id, sizeof(int));
		datastream.write((char*)&var.area, sizeof(int));
		datastream.write((char*)&var.mannings, sizeof(float));
		datastream.write((char*)&var.IL, sizeof(float));
		datastream.write((char*)&var.CL, sizeof(float));
		datastream.write((char*)&var.avgslope, sizeof(float));
		datastream.write((char*)&var.flowdistance, sizeof(float));
		datastream.write((char*)&var.highestpt, sizeof(float));
		datastream.write((char*)&var.lowestpt, sizeof(float));
		datastream.write((char*)&var.longestfplength, sizeof(float));
		datastream.write((char*)&var.dp, sizeof(DischargePoint));
		datastream.write((char*)&var.bounds, sizeof(MinMax));

		//Longest FP
		datastream.write((char*)&var.longest.id, sizeof(int));
		int size = var.longest.path.size();
		datastream.write((char*)&size, sizeof(int));
		for (int i = 0; i < size; i++)
		{
			datastream.write((char*)&var.longest.path[i], sizeof(Vec2));
		}

		//Catchment BDY
		size = var.points.size();
		datastream.write((char*)&size, sizeof(int));
		for (int i = 0; i < size; i++)
		{
			datastream.write((char*)&var.points[i], sizeof(Vec2));
		}

		//Isochrone Areas
		size = var.isochroneareas.size();
		datastream.write((char*)&size, sizeof(int));
		for (int i = 0; i < size; i++)
		{
			datastream.write((char*)&var.isochroneareas[i], sizeof(int));
		}
	}

	datastream.close();
}

void FileWriter::WriteTimeSeriestoCSV(std::string filepath, RainfallSeries rs)
{
	std::ofstream outfile(filepath);

	for (auto p : rs.series)
	{
		std::string line = std::to_string(p.time) + "," + std::to_string(p.value);
		outfile << line << std::endl;
	}
	outfile.close();
}

void FileWriter::WriteTimeSeriestoCSV(std::string filepath, FlowSeries fs)
{
	std::ofstream outfile(filepath);

	outfile << "Time(mins),Flow(cumecs),\n";

	for (auto p : fs.series)
	{
		std::string line = std::to_string(p.time) + "," + std::to_string(p.flow) + ",";
		outfile << line << std::endl;
	}
	outfile.close();
}
