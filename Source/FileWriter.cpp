#include "../Headers/FileWriter.h"
#include <fstream>

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
		for (int i = 0; quad.bottomnodes.size(); i++)
		{
			std::ofstream outfile(filename + std::to_string(i) + ".asc");

			double boundsx = (quad.bottomnodes[i]->BottomRight().x) - (quad.bottomnodes[i]->TopLeft().x);
			double boundsy = (quad.bottomnodes[i]->TopLeft().y) - (quad.bottomnodes[i]->BottomRight().y);
			double bottom = (quad.bottomnodes[i]->BottomRight().y);
			double left = (quad.bottomnodes[i]->TopLeft().x);
			int ncols = int(boundsx);
			int nrows = int(boundsy);

			outfile << std::fixed;
			outfile << "ncols\t\t\t" << (ncols) << "\n";
			outfile << "nrows\t\t\t" << (nrows) << "\n";
			outfile << "xllcorner\t\t" << left << "\n";
			outfile << "yllcorner\t\t" << bottom << "\n";
			outfile << "cellsize\t\t" << quad.spacing << "\n";
			outfile << "NODATA_value\t" << -9999 << "\n ";

			for (int y = 0; y < boundsy; y++)
			{
				for (int x = 0; x < boundsx; x++)
				{
					auto row = (boundsy - y) + bottom;
					auto column = x + left;

					auto n = quad.Search(Coordinates(column, row));

					if ( n != nullptr)
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
	}

}

void FileWriter::WriteAccumTreeASC(std::string filename, QuadtreeManager<FlowAccumulation>& quad)
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
				if ((quad.Search(FlowAccumulation(column, row))) != nullptr)
				{
					FlowAccumulation c = quad.Search(FlowAccumulation(column, row))->pos;
					std::string line = std::to_string(c.flow) + " ";
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

void FileWriter::WriteVecNormals3d(std::string filename, Quadtree<Normal>* normals) {
   // std::ofstream outfile(path + filename + ".csv");

   // float boundsx = (normals->BottomRight().x) - (normals->TopLeft().x);
   // float boundsy = (normals->TopLeft().y) - (normals->BottomRight().y);
   // float bottom = (normals->BottomRight().y);
   // float left = (normals->TopLeft().x);

   // for (int x = 0; x <= boundsx; x++)
   //     for (int y = 0; y <= boundsy; y++)
   //     {
   //         auto c = normals->search(Normal(x + left, y + bottom))->pos;
   //         std::string line = std::to_string(c.x) + "," + std::to_string(c.y) + "," + std::to_string(c.z) + "," + std::to_string(c.norm.x) + "," + std::to_string(c.norm.y) + "," + std::to_string(c.norm.z) + ",";
			//outfile << line << std::endl;
   //     };
}

void FileWriter::WriteVecNormals2d(std::string filename, Quadtree<Normal>* normals) {
	//std::ofstream outfile(path + filename + ".csv");

	//float boundsx = (normals->BottomRight().x) - (normals->TopLeft().x);
	//float boundsy = (normals->TopLeft().y) - (normals->BottomRight().y);
	//float bottom = (normals->BottomRight().y);
	//float left = (normals->TopLeft().x);

	//for (int x = 0; x <= boundsx; x++)
	//	for (int y = 0; y <= boundsy; y++)
	//	{
	//		auto c = normals->search(Normal(x + left, y + bottom))->pos;
	//		std::string line = std::to_string(c.x) + "," + std::to_string(c.y) + "," + std::to_string(c.norm.x) + "," + std::to_string(c.norm.y) + ",";
	//		outfile << line << std::endl;
	//	};
}

void FileWriter::WriteVecNormals3dWKT(std::string filename, QuadtreeManager<Normal>& normals) {
	if (normals.type == TreeType::Single)
	{
		std::ofstream outfile(filename + ".csv");

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

		float boundsx = (normals.BottomRight().x) - (normals.TopLeft().x);
		float boundsy = (normals.TopLeft().y) - (normals.BottomRight().y);
		float bottom = (normals.BottomRight().y);
		float left = (normals.TopLeft().x);

		for (int x = 0; x <= boundsx; x++)
			for (int y = 0; y <= boundsy; y++)
			{
				auto c = normals.Search(Normal(x + left, y + bottom))->pos;
				std::string line = "LINESTRING (" + std::to_string(c.x) + " " + std::to_string(c.y) + "," + std::to_string(c.norm.x) + " " + std::to_string(c.norm.y) + ")";
				outfile << line << std::endl;
			}
		outfile.close();
	}
	else
	{

	}
}

void FileWriter::WriteStreamPaths2dWKT(std::string filename, std::vector<std::vector<Vec2>>& flowpaths)
{
	std::ofstream outfile(filename + ".csv");

	for each (std::vector<Vec2> list in flowpaths)
	{
		std::string line = "LINESTRING (";

		for each (Vec2 point in list)
		{
			line += std::to_string(point.x) + " " + std::to_string(point.y) + ",";
		}
		line.erase(line.size() - 1, 1);
		line += ")";
		outfile << line << std::endl;
	}
	outfile.close();
}
