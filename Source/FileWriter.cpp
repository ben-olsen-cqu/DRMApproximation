#include "../Headers/FileWriter.h"
#include <fstream>

#if defined(WINDOWS) || defined(LINUX)
std::string path = "./Export Data/";
#elif defined(MAC)
std::string path = "/Users/ben/Documents/source/repos/Catchment_Creator/Catchment_Creator/Catchment Creator/Export Data/";
#endif

void FileWriter::WriteFiles(std::string filename, std::vector<std::string> data, bool multiplefiles)
{
	if (multiplefiles)
	{
		for (int i = 0; i < data.size(); i++)
			Write(path + filename + std::to_string(i) + ".csv", data[i]);
	}
	else
	{
		Write(path + filename + ".csv", data);
	}
}

void FileWriter::WriteCoordTree(std::string filename, Quadtree<Coordinates>* quad)
{
 //   std::ofstream outfile(path + filename + ".csv");

	//float boundsx = (quad->BottomRight().x) - (quad->TopLeft().x);
	//float boundsy = (quad->TopLeft().y) - (quad->BottomRight().y);
	//float bottom = (quad->BottomRight().y);
	//float left = (quad->TopLeft().x);

	//for (int x = 0; x <= boundsx; x++)
	//	for (int y = 0; y <= boundsy; y++)
	//	{
	//		Coordinates c = quad->search(Coordinates(x + left, y + bottom))->pos;
	//		std::string line = std::to_string(c.x) + "," + std::to_string(c.y) + "," + std::to_string(c.z) + ",";
 //           outfile << line << std::endl;
	//	}
}

void FileWriter::WriteCoordTreeASC(std::string filename, const Quadtree<Coordinates>* quad)
{
	//std::ofstream outfile(path + filename + ".asc");

	//float boundsx = (quad->BottomRight().x) - (quad->TopLeft().x);
	//float boundsy = (quad->TopLeft().y) - (quad->BottomRight().y);
	//float bottom = (quad->BottomRight().y);
	//float left = (quad->TopLeft().x);
	//int ncols = int(boundsx);
	//int nrows = int(boundsy);

	//outfile << std::fixed;
	//outfile << "ncols\t\t\t" << (ncols) << "\n";
	//outfile << "nrows\t\t\t" << (nrows) << "\n";
	//outfile << "xllcorner\t\t" << left << "\n";
	//outfile << "yllcorner\t\t" << bottom << "\n";
	//outfile << "cellsize\t\t" << 1.0f << "\n";
	//outfile << "NODATA_value\t" << -9999 << "\n ";

	//for (int y = 0; y < boundsy; y++)
	//{
	//	for (int x = 0; x < boundsx; x++)
	//	{
	//		auto row = (boundsy - y) + bottom;
	//		auto column = x + left;
	//		if ((quad->search(Coordinates(column, row))) != nullptr)
	//		{
	//			Coordinates c = quad->search(Coordinates(column, row))->pos;
	//			std::string line = std::to_string(c.z) + " ";
	//			outfile << line;
	//		}
	//	}
	//	outfile << std::endl << " ";
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

void FileWriter::WriteVecNormals3dWKT(std::string filename, Quadtree<Normal>* normals) {
	//std::ofstream outfile(path + filename + ".csv");

	//float boundsx = (normals->BottomRight().x) - (normals->TopLeft().x);
	//float boundsy = (normals->TopLeft().y) - (normals->BottomRight().y);
	//float bottom = (normals->BottomRight().y);
	//float left = (normals->TopLeft().x);

	//for (int x = 0; x <= boundsx; x++)
	//	for (int y = 0; y <= boundsy; y++)
	//	{
	//		auto c = normals->search(Normal(x + left, y + bottom))->pos;
	//		std::string line = "LINESTRING (" + std::to_string(c.x) + " " + std::to_string(c.y) + " " + std::to_string(c.z) + ","
	//			+ std::to_string(c.norm.x) + " " + std::to_string(c.norm.y) + " " + std::to_string(c.norm.z) + ")";
	//		outfile << line << std::endl;
	//	};
}

void FileWriter::WriteVecNormals2dWKT(std::string filename, Quadtree<Normal>* normals) {
	//std::ofstream outfile(path + filename + ".csv");

	//float boundsx = (normals->BottomRight().x) - (normals->TopLeft().x);
	//float boundsy = (normals->TopLeft().y) - (normals->BottomRight().y);
	//float bottom = (normals->BottomRight().y);
	//float left = (normals->TopLeft().x);

	//for (int x = 0; x <= boundsx; x++)
	//	for (int y = 0; y <= boundsy; y++)
	//	{
	//		auto c = normals->search(Normal(x + left, y + bottom))->pos;
	//		std::string line = "LINESTRING (" + std::to_string(c.x) + " " + std::to_string(c.y) + "," + std::to_string(c.norm.x) + " " + std::to_string(c.norm.y) + ")";
	//		outfile << line << std::endl;
	//	};
}